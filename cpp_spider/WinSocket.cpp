# include							"WinSocket.h"

const std::string					&WinSocket::getMachineIp()
{
	static std::string				ip = "";

	if (ip.size() == 0) {
		if (WSAStartup(MAKEWORD(2, 2), &this->_wsaData) == 0)
			throw BBException("WSAStartup failed");
		char szHostName[255];
		gethostname(szHostName, 255);
		struct hostent *host_entry;
		host_entry = gethostbyname(szHostName);
		ip = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);
		WSACleanup();
	}
	return (const_cast<const std::string &>(ip));
}

void							WinSocket::launchWSA()
{
	if (WSAStartup(MAKEWORD(2, 2), &this->_wsaData) != 0)
		throw BBException("WSAStartup failed");
	this->_socket = INVALID_SOCKET;
	this->_status = ISocket::Status::Waiting;
	this->_addrinfo = NULL;
}

void							WinSocket::acceptNewClients(unsigned int  thread_id, WinSocket *server)
{
	WinSocket					*newClient;
	struct sockaddr_in			s_in;
	int							s_inLength = sizeof(s_in);

	server->_status = ISocket::Status::Running;
	while (server->_status == ISocket::Status::Running)
	{
		newClient = new WinSocket(ISocket::Type::Client);
		if ((newClient->_socket = accept(server->_socket, reinterpret_cast<struct sockaddr *>(&s_in), &s_inLength)) == INVALID_SOCKET)
		{
			closesocket(newClient->_socket);
			server->_status = ISocket::Status::Canceled;
		}
		else
		{
			newClient->_socketOpened = true;
			newClient->_status = ISocket::Ready;
			newClient->_port = s_in.sin_port;
			newClient->_ip = inet_ntoa(s_in.sin_addr);
			server->addNewClient(newClient);
			newClient->_thread = new WinThread<void, WinSocket *>(WinSocket::launchClient);
			(*newClient->_thread)(newClient);
		}
	}
}

void							WinSocket::launchClient(unsigned int thread_id, WinSocket *client)
{
	MutexVault					*vault = MutexVault::getMutexVault();
	WSAPOLLFD					ufd[2];
	char						buffer[READ_HEAP];
	int							pollReturn;
	int							sendReturn;
	int							recvReturn;

	client->_status = ISocket::Status::Running;
	ufd[0].fd = (ufd[1].fd = client->_socket);
	ufd[0].events = POLLIN;
	ufd[1].events = POLLOUT;
	if (client->getOnConnect() != NULL)
		client->getOnConnect()(client);
	while (client->getStatus() != ISocket::Status::Canceled)
	{
		if ((pollReturn = WSAPoll(&ufd[0], 1, TIMEOUT)) == SOCKET_ERROR)
			client->_status = ISocket::Status::Canceled;
		else if (pollReturn > 0)
		{
			(*vault)["read" + MutexVault::toString(client->getId())]->lock(true);
			if (client->_read_buffer.size() > MAX_BUFFER_SIZE)
				client->_read_buffer.clear();
			if ((recvReturn = recv(client->_socket, &buffer[0], READ_HEAP, 0)) == SOCKET_ERROR)
				client->_status = ISocket::Canceled;
			else if (recvReturn == 0)
				client->_status = ISocket::Canceled;
			else {
				for (int i = 0; i < recvReturn; i++)
					client->_read_buffer.push_back(buffer[i]);
				//while protected -> summon onReceive
				if (client->getOnReceive() != NULL)
					client->getOnReceive()(client);
			}
			(*vault)["read" + MutexVault::toString(client->getId())]->unlock();
		}
		else if (client->getStatus() != ISocket::Canceled && !client->_write_buffer.empty())
		{
			if ((pollReturn = WSAPoll(&ufd[1], 1, TIMEOUT)) == SOCKET_ERROR)
				client->_status = ISocket::Canceled;
			if (pollReturn > 0)
			{
				//begin protected action
				(*vault)["write" + MutexVault::toString(client->getId())]->lock(true);
				if ((sendReturn = send(client->_socket, reinterpret_cast<char *>(&(client->_write_buffer[0])), client->_write_buffer.size(), 0)) == SOCKET_ERROR)
					client->_status = ISocket::Canceled;
				//delete sent data
				if (sendReturn > 0)
					client->_write_buffer.erase(client->_write_buffer.begin(), client->_write_buffer.begin() + sendReturn);
				(*vault)["write" + MutexVault::toString(client->getId())]->unlock();
				//end protected action
			}
			//no event
		}
	}
	client->cancel();
	if (client->getOnDisconnect() != NULL)
		client->getOnDisconnect()(client);
}

// DUMMY
WinSocket::WinSocket(ISocket::Type type) : ISocket(type)
{
	this->_socketOpened = false;
}

// SERVER
WinSocket::WinSocket(int port, const std::string &proto) : ISocket(ISocket::Server, WinSocket::getMachineIp(), port)
{
	struct protoent				*pe;
	struct addrinfo				hints;

	if (this->_ip.empty())
		throw BBException("No ip detected!");
	if ((pe = getprotobyname(proto.c_str())) == NULL)
		throw BBException("getprotobyname failed");

	this->launchWSA();
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // test with AF_UNSPEC later
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = pe->p_proto;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &this->_addrinfo) != 0)
	{
		WSACleanup();
		throw BBException("getaddrinfo failed");
	}

	if ((this->_socket = socket(this->_addrinfo->ai_family, this->_addrinfo->ai_socktype, this->_addrinfo->ai_protocol)) == INVALID_SOCKET ||
		bind(this->_socket, this->_addrinfo->ai_addr, static_cast<int>(this->_addrinfo->ai_addrlen)) == SOCKET_ERROR ||
		listen(this->_socket, MAX_CLIENTS) == SOCKET_ERROR)
	{
		freeaddrinfo(this->_addrinfo);
		WSACleanup();
		throw BBException("Error on socket(), listen() or bind(): " + WSAGetLastError());
	}
	freeaddrinfo(this->_addrinfo); // no longer needed
	this->_status = ISocket::Status::Ready;
}

// CLIENT
WinSocket::WinSocket(const std::string &ip, int port, const std::string &proto) : ISocket(ISocket::Client, ip, port)
{
	struct protoent				*pe;
	struct addrinfo				hints;

	//if ((pe = getprotobyname(proto.c_str())) == NULL)
		//throw BBException("getprotobyname failed");

	this->launchWSA();
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &this->_addrinfo) != 0)
	{
		WSACleanup();
		throw BBException("getaddrinfo failed");
	}

	this->_socket = socket(this->_addrinfo->ai_family, this->_addrinfo->ai_socktype, this->_addrinfo->ai_protocol);
	if (this->_socket == INVALID_SOCKET)
	{
		freeaddrinfo(this->_addrinfo);
		WSACleanup();
		throw BBException("Error at socket(): " + WSAGetLastError());
	}
	this->_status = ISocket::Status::Ready;
}

void							WinSocket::cancel()
{
	IMutex						*mutex;

	mutex = (*MutexVault::getMutexVault())["close" + MutexVault::toString(this->_id)];

	if (this->_type == ISocket::Type::Server)
	{
		std::vector<ISocket *> activeClients = this->getActiveClients();
		std::cout << "disconnecting " << activeClients.size() << " clients" << std::endl;
		for (unsigned int i = 0; i < activeClients.size(); i++)
			activeClients[i]->cancel();
	}
	mutex->lock(true);
	if (this->_socket != INVALID_SOCKET)
		closesocket(this->_socket);
	this->_socket = INVALID_SOCKET;
	this->_socketOpened = false;
	mutex->unlock();
	this->_status = ISocket::Status::Canceled;
}

int								WinSocket::start()
{
	if (this->_status != ISocket::Status::Ready)
		return (0);
	if (this->_type == ISocket::Type::Server)
	{
		this->_thread = new WinThread<void, WinSocket *>(WinSocket::acceptNewClients);
		(*this->_thread)(this);
	}
	else if (this->_type == ISocket::Type::Client)
	{
		if (connect(this->_socket, this->_addrinfo->ai_addr, static_cast<int>(this->_addrinfo->ai_addrlen)) == SOCKET_ERROR)
		{
			this->cancel();
			return (-1);
		}
		this->_thread = new WinThread<void, WinSocket *>(WinSocket::launchClient);
		(*this->_thread)(this);
	}
	return (1);
}