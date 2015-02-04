#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <cerrno>

#include "exceptions.h"
#include "server.h"
#include "message.h"

#define BUFFER_SIZE 1024

using namespace std;

Server::Server(int argc, char **argv) {
	port = 0;

	if (argc == 2)
		stringstream(argv[1]) >> port;
}

void Server::setup() {
	answering_machine = socket(AF_INET, SOCK_STREAM, 0);

	if (answering_machine == -1)
		throw SOCK_EX;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if (bind(answering_machine, (const sockaddr*)&address, sizeof(address)) == -1)
		throw BIND_EX;

	if (listen(answering_machine, 10) == -1)
		throw LISTEN_EX;

	num_open = 1;
	capacity = BUFFER_SIZE;
	open_sockets = (int *) calloc(capacity, sizeof(int));
	open_sockets[0] = answering_machine;
	clients = (char **) calloc(capacity, sizeof(char *));
	clients[0] = (char *)calloc(1, sizeof(char));
	clients[0][0] = '\0';
}

void Server::serve() {	
	// service loop
	while(true) {
		cout << "Waiting for clients..\n";
		
		//setup 
		FD_ZERO(&read_set);
		wait_time.tv_sec = 1;
		wait_time.tv_usec = 1000;
	
		for (int i = 0; i < num_open; i++) {
			current_socket = open_sockets[i];
			FD_SET(current_socket, &read_set);
		}
	
		//select
		if (select(open_sockets[num_open - 1] + 1, &read_set, NULL, NULL, &wait_time) == -1)
			throw SELECT_EX;

		//respond
		for (int i = 0; i < num_open; i++) {
			current_socket = open_sockets[i];
			if (FD_ISSET(current_socket, &read_set)) {
				
				//setup new connection
				if (current_socket == answering_machine) {
					answer();
				}
				//receive from a client
				else {
					read_packet(current_socket);
				}
			}
		}
	}
}

void Server::shutdown() {
	
}

void Server::answer() {
	resize();
	
	open_sockets[num_open++] = accept(answering_machine, NULL, 0);
	
	if (open_sockets[num_open] == -1)
		throw ACCEPT_EX;
}

void Server::read_packet(int fd) {
	uint8_t *buffer;
	int message_size;
	Message *msg;

	buffer = (uint8_t *) calloc(BUFFER_SIZE, sizeof(uint8_t));

	message_size = recv(fd, buffer, BUFFER_SIZE, 0);

	if (message_size) {
		
		for (int i = 0; i < message_size; i++)
			printf("%c", buffer[i]);
		printf("\n");
		
		msg = new Message(buffer, message_size);
		msg->print();
		
		if (msg->get_flag() == 1) {
			try {
				learn_handle(fd, msg->get_from(), msg->get_from_length());
				confirm_handle(msg->get_from(), msg->get_from_length());
			}
			catch (...) {
				printf("Exception caught during name learning");
			}
		}
		
	}
	else {
		cout << "Connection terminated.\n";
		close_connection(fd);
	}

	free(buffer);
}

void Server::learn_handle(int fd, char *handle, int length) {
	int index = 0;
	
	for (int i = 0; i < num_open; i++) 
		if (open_sockets[i] == fd)
			index = i;
	
	if (handle[0] == '\0' || handle_exists(handle, length)) {
		bad_handle_response(handle, length);
		return;
	}	
	
	if (clients[index] != NULL)
		free(clients[index]);
	
	clients[index] = (char *) calloc(length + 1, sizeof(char));
	memcpy(clients[index], handle, length);
	clients[index][length] = '\0';
}

void Server::confirm_handle(char *to, int to_length) {
	Message *msg;
	int client_fd, error, msg_size;
	
	msg = new Message();
	msg->set_to(to, to_length);
	msg->set_flag(2);
	msg_size = msg->pack();
	
	client_fd = find_by_handle(to, to_length);
		
	error = send(client_fd, msg->sendable(), msg_size, 0);
		
	if (error == -1) {
		throw SEND_EX;
	}

	delete msg;
}

int Server::handle_exists(char *handle, int length) {
	char *handle_str;
	
	handle_str = (char *) calloc(length + 1, sizeof(char));
	memcpy(handle_str, handle, length);
	for (int i = 0; i < num_open; i++) {
		if (strcmp(handle_str, clients[i]) == 0) {
			free(handle_str);
			return i;
		}
	}
	
	free(handle_str);
	return 0;
	
}

void Server::bad_handle_response(char *handle, int length) {
	Message *msg;
	int client_fd, error, msg_size;
	
	msg = new Message();
	msg->set_to(handle, length);
	msg->set_flag(3);
	msg_size = msg->pack();
	
	client_fd = find_by_handle(handle, length);
		
	error = send(client_fd, msg->sendable(), msg_size, 0);
		
	if (error == -1) {
		throw SEND_EX;
	}

	delete msg;
}

int Server::find_by_handle(char *handle, int length) {
	char *handle_str;
	
	handle_str = (char *) calloc(length + 1, sizeof(char));
	memcpy(handle_str, handle, length);
	for (int i = 0; i < num_open; i++) {
		if (strcmp(handle_str, clients[i]) == 0) {
			free(handle_str);
			return open_sockets[i];
		}
	}
	
	free(handle_str);
	throw HANDLE_EX;
}

void Server::close_connection(int fd) {
	close(fd);
	for (int i = 0; i < num_open; i++) {
		if (open_sockets[i] == fd) {
			open_sockets[i] = open_sockets[--num_open];
			if (clients[i] != NULL) {
				free(clients[i]);
				clients[i] = clients[num_open];
				clients[num_open] = NULL;
			}
			break;
		}
	}
}

void Server::resize() {
	int *new_sockets;
	char **new_clients;

	if (num_open == capacity - 2) {
		new_sockets = (int *)calloc(2*capacity, sizeof(int));
		new_clients = (char **)calloc(2*capacity, sizeof(char *));
		memcpy(new_sockets, open_sockets, capacity);
		memcpy(new_clients, clients, capacity);
		free(open_sockets);
		free(clients);
		open_sockets = new_sockets;
		clients = new_clients;
		capacity = 2*capacity;
	}
}