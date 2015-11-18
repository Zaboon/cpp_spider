//
// Created by rustam_t on 10/19/15.
//

#ifndef CPP_BABEL_LINUXSOCKET_H
#define CPP_BABEL_LINUXSOCKET_H

#include <algorithm>
#include <exception>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <stdio.h>
#include "LinuxThread.hpp"
#include "../ISocket.h"
#include "../BBException.hpp"

#define MAX_CLIENTS 42
#define TIMEOUT 50
#define READ_HEAP 40800
#define MAX_BUFFER_SIZE 400960

class LinuxSocket : public ISocket
{
private:

    int                                 _socket;
    bool                                _socketOpened;
    LinuxThread<void, LinuxSocket *>    *_thread;

protected:

    static void acceptNewClients(unsigned int thread_id, LinuxSocket *server);

    static void launchClient(unsigned int thread_id, LinuxSocket *client);

    //dummy
    LinuxSocket(ISocket::Type type);

public:

    //Client
    LinuxSocket(const std::string &ip, int port = 42, const std::string &proto = "TCP");

    //Server
    LinuxSocket(int port = 4242, const std::string &proto = "TCP");

    static const std::string &getMachineIp();

    virtual int start();

    void cancel();
};


#endif //CPP_BABEL_LINUXSOCKET_H
