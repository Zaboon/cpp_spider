//
// Created by rustam_t on 10/19/15.
//

#include "LinuxSocket.h"

const std::string &
LinuxSocket::getMachineIp()
{
    static std::string ip;

    if (ip.size() == 0) {

        char buffer[128];
        FILE *pipe = popen("hostname -i", "r");

        if (pipe) {
            if (fgets(buffer, 128, pipe) != NULL)
                ip += buffer;
            pclose(pipe);
            //erase whitespace and \n
            ip.erase(std::remove(ip.begin(), ip.end(), '\n'), ip.end());
            ip.erase(std::remove(ip.begin(), ip.end(), ' '), ip.end());
        }
    }

    return (const_cast<const std::string &>(ip));
}

void
LinuxSocket::cancel()
{
    IMutex *mutex = (*MutexVault::getMutexVault())["close" + MutexVault::toString(this->_id)];

    if (this->_type == ISocket::Server) {

        std::vector<ISocket *> activeClients = this->getActiveClients();
        std::cout << "disconnecting " << activeClients.size() << " clients" << std::endl;
        for (unsigned int i = 0; i < activeClients.size(); i++)
            activeClients[i]->cancel();
    }
    mutex->lock(true);
    if (this->_socketOpened)
    {
        shutdown(this->_socket, 0);
        close(this->_socket);
        this->_socketOpened = false;
    }
    mutex->unlock();
    this->_status = ISocket::Canceled;
}

void
LinuxSocket::acceptNewClients(unsigned int __attribute__((__unused__)) thread_id, LinuxSocket *server)
{
    struct sockaddr_in    s_in;
    socklen_t clinen = sizeof(s_in);
    LinuxSocket *newclient;

    server->_status = ISocket::Running;
    while (server->_status == ISocket::Running)
    {
        newclient = new LinuxSocket(ISocket::Client);
        if ((newclient->_socket = accept(server->_socket, reinterpret_cast<struct sockaddr *>(&s_in), &clinen)) < 0)
            server->_status = ISocket::Canceled;
        else {
            newclient->_socketOpened = true;
            newclient->_status = ISocket::Ready;
            newclient->_port = s_in.sin_port;
            newclient->_ip = inet_ntoa(s_in.sin_addr);
            server->addNewClient(newclient);
            newclient->_thread = new LinuxThread<void, LinuxSocket *>(LinuxSocket::launchClient);
            (*newclient->_thread)(newclient);
        }
    }
}

void
LinuxSocket::launchClient(unsigned int __attribute__((__unused__)) thread_id, LinuxSocket *client)
{
    struct pollfd ufd[2];
    unsigned char buffer[READ_HEAP];
    int rv;
    ssize_t send_val;
    ssize_t read_val;
    ISocket::EventHandler handler[3];

    MutexVault *vault = MutexVault::getMutexVault();
    client->_status = ISocket::Running;

    ufd[0].fd = (ufd[1].fd = client->_socket);
    ufd[0].events = POLLIN;
    ufd[1].events = POLLOUT;

    IMutex *read = (*MutexVault::getMutexVault())["read" + MutexVault::toString(client->getId())];
    IMutex *write = (*MutexVault::getMutexVault())["write" + MutexVault::toString(client->getId())];

    //if onConnect is attached
    if ((handler[0] = client->getOnConnect()) != NULL)
        handler[0](client);

    while (client->getStatus() != ISocket::Canceled)
    {
        if ((rv = poll(&ufd[0], 1, TIMEOUT)) == -1)
            client->_status = ISocket::Canceled;
        read->lock(true);
        if (rv > 0)
        {
            //begin protected action
            if (client->_read_buffer.size() > MAX_BUFFER_SIZE)
                client->_read_buffer.clear();
            if ((read_val = recv(client->_socket, &buffer[0], READ_HEAP, 0)) == -1)
                client->_status = ISocket::Canceled;
            else if (read_val == 0)
                client->_status = ISocket::Canceled;
            else {
                for (unsigned int i = 0; i < read_val; i++)
                    client->_read_buffer.push_back(buffer[i]);
            }
            //end protected action
        }
        //while protected -> summon onReceive
        if (client->_read_buffer.size() > 0 && (handler[1] = client->getOnReceive()) != NULL)
            handler[1](client);
        read->unlock();


        if ((rv = poll(&ufd[1], 1, TIMEOUT)) == -1)
            client->_status = ISocket::Canceled;
        if (rv > 0)
        {
            //begin protected action
            write->lock(true);
            if (client->_write_buffer.size() > 0) {

                if ((send_val = send(client->_socket, &(client->_write_buffer[0]), client->_write_buffer.size(), 0)) == -1)
                    client->_status = ISocket::Canceled;
                else if (send_val > 0)
                    client->_write_buffer.erase(client->_write_buffer.begin(), client->_write_buffer.begin() + send_val);
            }
            write->unlock();
            //end protected action
        }
        //no event
    }
    client->cancel();
    if ((handler[2] = client->getOnDisconnect()) != NULL)
        handler[2](client);
}

//dummy
LinuxSocket::LinuxSocket(LinuxSocket::Type type) : ISocket(type)
{
    this->_socketOpened = false;
}

//Server
LinuxSocket::LinuxSocket(int port, const std::string &proto) : ISocket(ISocket::Server, LinuxSocket::getMachineIp(), port)
{
    struct protoent       *pe;
    struct sockaddr_in    s_in;

    if (this->_ip.empty())
        throw BBException("No ip detected!");
    this->_status = ISocket::Waiting;
    if ((pe = getprotobyname(proto.c_str())) == NULL)
        throw BBException("getprotobyname failed");
    if ((this->_socket = socket(AF_INET, SOCK_STREAM, pe->p_proto)) == -1)
        throw BBException("socket failed");
    this->_status = ISocket::Ready;
    s_in.sin_family = AF_INET;
    s_in.sin_port = htons(static_cast<uint16_t>(port));
    s_in.sin_addr.s_addr = INADDR_ANY;
    if (bind(this->_socket, reinterpret_cast<struct sockaddr *>(&s_in), sizeof(s_in)) == -1)
        throw BBException("bind failed");
    if (listen(this->_socket, MAX_CLIENTS) == -1)
        throw BBException("listen failed");
    this->_socketOpened = true;
}

int
LinuxSocket::start()
{
    if (this->_status != ISocket::Ready)
        return (0);
    if (this->_type == ISocket::Server) {
        //launch the thread to accept new connections
        this->_thread = new LinuxThread<void, LinuxSocket *>(LinuxSocket::acceptNewClients);
        (*this->_thread)(this);
    }
    else
    {
        struct sockaddr_in    s_in;

        s_in.sin_family = AF_INET;
        s_in.sin_port = htons(static_cast<uint16_t>(this->_port));
        if ((s_in.sin_addr.s_addr = inet_addr(this->_ip.c_str())) == INADDR_NONE ||
            (connect(this->_socket, reinterpret_cast<struct sockaddr *>(&s_in), sizeof(s_in))) == -1) {

            this->cancel();
            return (-1);
        }
        this->_thread = new LinuxThread<void, LinuxSocket *>(LinuxSocket::launchClient);
        (*this->_thread)(this);
    }
    return (1);
}

//Client
LinuxSocket::LinuxSocket(const std::string &ip, int port, const std::string &proto) : ISocket(ISocket::Client, ip, port)
{
    struct protoent       *pe;

    if ((pe = getprotobyname(proto.c_str())) == NULL)
        throw BBException("getprotobyname failed");
    if ((this->_socket = socket(AF_INET, SOCK_STREAM, pe->p_proto)) == -1)
        throw BBException("socket failed");
    this->_port = port;
    this->_ip = ip;
    this->_socketOpened = true;
    this->_status = ISocket::Ready;
}