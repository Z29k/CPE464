#ifndef CLIENT_H
#define CLIENT_H

class Client {
	char *handle;
	struct hostent *server;
	int port;
	int socket_fd;
	struct sockaddr_in server_address;
	bool exit;
	int sequence_number;
	
public:
	// Lifecycle
	Client(char *handle, struct hostent *server, int port);
	void init();
	void communicate();
	void janitor();

private:
	// Workflow
	void confirm_handle();
	void process_message();
	void process_user_input();
	int send_message(char *input);
	void broadcast(char *input);
	void list_handles();
	int  request_list_length();
	void request_handle(int index);
};

#endif
