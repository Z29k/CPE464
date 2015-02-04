#ifndef SERVER_H
#define SERVER_H

class Server {
	
	int answering_machine; // socket to linsten() on
	int current_socket;
	int *open_sockets; // open connections to clients
	char **clients; // array of client handles
	int num_open;
	int capacity;
	fd_set read_set;
	struct timeval wait_time;
	int port;
	struct sockaddr_in address;
	
public:
	Server(int argc, char **argv);
	
	void setup();
	void serve();
	void shutdown();
private: 

	void answer();
	void read_packet(int fd);
	void learn_handle(int fd, char *handle, int length);
	void confirm_handle(char *to, int to_length);
	void bad_handle_response(char *handle, int length);
	int  find_by_handle(char *handle, int length);
	int handle_exists(char *handle, int length);
	void close_connection(int fd);
	void resize();
};

#endif