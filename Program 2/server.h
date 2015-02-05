#ifndef SERVER_H
#define SERVER_H

#include "message.h"

class Server {
	
	int answering_machine; // socket to linsten() on
	int current_socket;
	int *open_sockets; // open connections to clients
	char **clients; // array of client handles
	int num_open;
	int num_clients;
	int capacity;
	int highest_fd;
	fd_set read_set;
	struct timeval wait_time;
	int port;
	struct sockaddr_in address;
	int unconfirmed_clients;
	
public: // Lifecycle
	Server(int argc, char **argv);
	void setup();
	void serve();
	void shutdown();
	
private: 
	// Workflow
	void process_packet(int fd);
	void answer();
	void learn_handle(int fd, char *handle, int length);
	void broadcast(Message *msg, int msg_size);
	void close_connection(int fd);
	
	// Responses to client
	void confirm_handle(char *to, int to_length);
	void bad_handle_response(int fd, char *handle, int length);
	void bad_destination_response(int fd, char *handle, int length);
	void exit_response(int fd);
	
	// Utility
	int  find_by_handle(char *handle, int length);
	int handle_exists(char *handle, int length);
	void resize();
	void print_tables();
};

#endif
