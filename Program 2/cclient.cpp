#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "message.h"
#include "exceptions.h"
#include "client.h"

#define ARG_EX 1
#define IP_EX 2
#define SOCK_EX 3
#define CONNECT_EX 4

#define BUFFER_SIZE 2000
#define INPUT_MAX 1150

using namespace std;

/*
 * Lifecycle
 */	
Client::Client(char *handle, struct hostent *server, int port) {
	this->handle = handle;
	this->server = server;
	this->port = port;
	exit = false;
}

void Client::init() {
	Message *msg;
	int msg_size, error;	
	
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons((short) port);
	
	memcpy(&(server_address.sin_addr.s_addr), server->h_addr, server->h_length);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd == -1)
		throw SOCK_EX;

	if (connect(socket_fd, (struct sockaddr *)(&server_address), 16) == -1)
		throw CONNECT_EX;
	
	
	msg = new Message();
	msg->set_flag(1);
	msg->set_from(handle, strlen(handle));
	msg_size = msg->pack();
	
	error = send(socket_fd, msg->sendable(), msg_size, 0);
	
	if (error == -1)
		throw SEND_EX;
	
	delete msg;
	
	confirm_handle();
}

void Client::communicate() {
	struct timeval wait_time;
	fd_set read_set;
	
	printf("> ");
	fflush(stdout);
	
	while (!exit) {
	
		FD_ZERO(&read_set);

		wait_time.tv_sec = 1;
		wait_time.tv_usec = 0;

		FD_SET(socket_fd, &read_set);
		FD_SET(0, &read_set);

		if (select(socket_fd + 1, &read_set, NULL, NULL, &wait_time) == -1)
			throw SELECT_EX;

		if (FD_ISSET(socket_fd, &read_set)) 
			process_message();
	
		if (FD_ISSET(0, &read_set))
			process_user_input();
	}
}

void Client::janitor() {
	Message *msg;
	uint8_t *buffer;
	int msg_size, error, flag;

	// send exit message to server
	msg = new Message();
	msg->set_flag(8);
	msg->set_from(handle, strlen(handle));
	msg_size = msg->pack();
	
	error = send(socket_fd, msg->sendable(), msg_size, 0);
	
	if (error == -1)
		throw SEND_EX;
	
	delete msg;
	
	// wait for ACK from server

	buffer = (uint8_t *) calloc(BUFFER_SIZE, sizeof(uint8_t));

	msg_size = recv(socket_fd, buffer, BUFFER_SIZE, 0);
	
	// connection closed before ACK
	if (msg_size == 0)
		throw CONNECT_EX;
	
	msg = new Message(buffer, msg_size);
	flag = msg->get_flag();
	free(buffer);
	delete msg;

	// Check flag for ACK
	if (flag == 9) {
		printf("Connection to server closed.\n");
	}
	else 
		throw EXIT_EX;
	
	close(socket_fd);
}

/*
 * Workflow
 */

void Client::confirm_handle() {
	uint8_t *buffer;
	int message_size, flag;
	Message *msg;

	buffer = (uint8_t *) calloc(BUFFER_SIZE, sizeof(uint8_t));

	message_size = recv(socket_fd, buffer, BUFFER_SIZE, 0);
	
	if (message_size == 0)
		throw CONNECT_EX;
	
	msg = new Message(buffer, message_size);
	flag = msg->get_flag();
	free(buffer);
	delete msg;

	if (flag == 2) 
		return;
	else if (flag == 3) 
		throw HANDLE_EX;
	else
		throw MYSTERY_EX;
}

void Client::process_user_input() {
	char input_buffer[BUFFER_SIZE];
	char sel;
	
	fgets(input_buffer, BUFFER_SIZE, stdin);
	
	if (strlen(input_buffer) > 1) {
		input_buffer[0] == '%' ? sel = input_buffer[1] : sel = input_buffer[0];
			
		if (sel == 'm' || sel == 'M') {
			send_message(input_buffer);
		}
		else if (sel == 'b' || sel == 'B') {
			broadcast(input_buffer);
		}
		else if (sel == 'l' || sel == 'L') {
			list_handles();
		}
		else if (sel == 'e' || sel == 'E') {
			exit = true;
		}
		else {
			printf("M - message\n");
			printf("B - broadcast\n");
			printf("L - list\n");
			printf("E - exit\n");
		}
	}
	else
		printf("%%<option> <handle> <message>\n");
	
	printf("> ");
	fflush(stdout);
}

void Client::process_message() {
	uint8_t *buffer;
	int message_size, flag;
	char handle[101];
	Message *msg;

	buffer = (uint8_t *) calloc(BUFFER_SIZE, sizeof(uint8_t));

	message_size = recv(socket_fd, buffer, BUFFER_SIZE, 0);
	
	// connection closed
	if (message_size == 0)
		throw CONNECT_EX;
	
	msg = new Message(buffer, message_size);
	
	flag = msg->get_flag();
	
	printf("\n");
	
	if (flag == 6)
		msg->print();
	else if (flag == 7) {
		memcpy(handle, msg->get_to(), msg->get_to_length());
		handle[msg->get_to_length()] = '\0';
		printf("The handle %s does not exist.\n", handle);
	}
	
	delete msg;
	free(buffer);
	
	printf("> ");
	fflush(stdout);

}

void Client::send_message(char *input) {
	char to[101];
	int to_length;
	char garbage[100];
	char *message_start;
	int message_size;
	Message *msg;
	int error;
	
	
	sscanf(input, "%s %100s", garbage, to);
	to_length = strlen(to);
	
	message_start = strstr(input, to) + strlen(to) + 1;
	
	for (message_size = 0; message_start[message_size] != '\0'; message_size++);
	
	msg = new Message();
	msg->set_to(to, to_length);
	msg->set_from(handle, strlen(handle));
	
	if (message_size == 0)
		msg->set_text("\n", 1);
	else
		msg->set_text(message_start, message_size);
	
	msg->set_flag(6);
	message_size = msg->pack();
	
	error = send(socket_fd, msg->sendable(), message_size, 0);
	
	if (error == -1)
		throw SEND_EX;
}

void Client::broadcast(char *input) {
	char garbage[100];
	char *message_start;
	int message_size;
	Message *msg;
	int error;
	
	
	sscanf(input, "%s", garbage);
	
	message_start = input + strlen(garbage) + 1;
	
	for (message_size = 0; message_start[message_size] != '\0'; message_size++);
	
	msg = new Message();
	msg->set_from(handle, strlen(handle));
	msg->set_text(message_start, message_size);
	msg->set_flag(6);
	message_size = msg->pack();
	
	error = send(socket_fd, msg->sendable(), message_size, 0);
	
	if (error == -1)
		throw SEND_EX;
}

void Client::list_handles() {

}

