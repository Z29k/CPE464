#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <cerrno>

#include "message.h"
#include "exceptions.h"

#define ARG_EX 1
#define IP_EX 2
#define SOCK_EX 3
#define CONNECT_EX 4

using namespace std;

int main(int argc, char **argv) {
	char *handle;
	struct hostent *server;
	int port;
	int socket_fd;
	Message *msg;
	int msg_size;
	struct sockaddr_in server_address;
	
	try {
		if (argc != 4)
			throw ARG_EX;
			
		handle = argv[1];
		server = gethostbyname(argv[2]);
		stringstream(argv[3]) >> port;
	
		if (server == NULL)
			throw IP_EX;
	
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons((short) port);
		
		memcpy(&(server_address.sin_addr.s_addr), server->h_addr, server->h_length);
		
		cout << handle << "\n" << argv[2] << "\n" << port << "\n";
	
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	
		if (socket_fd == -1)
			throw SOCK_EX;
	
		if (connect(socket_fd, (struct sockaddr *)(&server_address), 16) == -1)
			throw CONNECT_EX;
	
		
		msg = new Message("Hello server!");
		//msg_size = msg->pack();
		//send(socket_fd, msg->sendable(), msg_size, 0);
	}
	catch(int ex) {
		if (ex == ARG_EX)
			cout << "cclient <handle> <server-IP> <server-port>\n";
		else if (ex == IP_EX) 
			cout << "Could not resolve hostname " << argv[2] << "\n";
		else if (ex == SOCK_EX)
			cout << "socket() failed. errno " << errno << "\n";
		else if (ex == CONNECT_EX)
			cout << "connect() failed. errno " << errno << "\n";
		else
			cout << "Unknown exception.\n";
	}
}
