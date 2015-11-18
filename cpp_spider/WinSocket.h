#ifndef CPP_BABEL_WINSOCKET_H_
# define CPP_BABEL_WINSOCKET_H_

# pragma comment(lib, "Ws2_32.lib")
# define _WINSOCK_DEPRECATED_NO_WARNINGS

# include							"ISocket.h"
# include							<socketapi.h>
# include							<WinSock2.h>
# include							<WS2tcpip.h>
# include							<MSWSock.h>
# include							"WinThread.hpp"
# include							"BBException.hpp"

# define MAX_CLIENTS				42
# define MAX_BUFFER_SIZE			40960
# define READ_HEAP					21920
# define TIMEOUT					5

class WinSocket :					public ISocket
{
private:
	WSADATA							_wsaData;
	SOCKET							_socket;
	struct addrinfo					*_addrinfo;
	bool							_socketOpened;
	WinThread<void, WinSocket *>    *_thread;

protected:
	static void							acceptNewClients(unsigned int  thread_id, WinSocket *server);
	static void							launchClient(unsigned int thread_id, WinSocket *client);
	WinSocket(ISocket::Type type);
	void							launchWSA();

public:
	const std::string				&getMachineIp();

	WinSocket(int port, const std::string &proto);
	WinSocket(const std::string &ip, int port, const std::string &proto);

	void							cancel();
	int								start();
};

#endif /* !CPP_BABEL_WINSOCKET_H_ */