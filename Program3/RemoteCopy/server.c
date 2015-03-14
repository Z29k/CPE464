/* server
	Works with Rcopy, sends a requested file
	
	To run: server errorRate
	
	By: Hugh Smith */
	
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "cpe464.h"
#include "window.h"
#include "packet.h"

typedef enum State STATE;

enum State {
	START, DONE, FILENAME, SEND_DATA, PROCESS, WAIT_ON_ACK,
};

void process_client(int32_t server_sk_num, uint8_t *buf, int32_t recv_len, Connection *client,
	Packet *packet);

STATE filename(Connection *client, Packet *packet, uint8_t *buf, int32_t recv_len, int32_t *data_file, 
	int32_t *bufsize, Window *window);

STATE send_data(Connection *client, Packet *packet, int32_t data_file, 
	int32_t buf_size, int32_t *seq_num, Window *window);

STATE process(Connection *client, Window *window);

STATE wait_on_ack(Connection *client, Window *window);

int main(int argc, char **argv) {
	int32_t server_sk_num = 0;
	pid_t pid = 0;
	int status = 0;
	uint8_t buf[MAX_LEN];
	Connection client;
	int32_t recv_len = 0;
	Packet packet;
	
	//struct sockaddr_in local; //socket address for us
	//uint32_t len = sizeof(local);
	
	if (argc != 2) {
		printf("Usage %s error_rate\n", argv[0]);
		exit(-1);
	}
	
	sendtoErr_init(atof(argv[1]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
	
	/*set up the main server port */
	server_sk_num = udp_server();
	
	while(1) {
		if (select_call(server_sk_num, 1, 0, NOT_NULL) == 1) {
			//recv_len = recv_buf(buf, 1000, server_sk_num, &client, &flag, &seq_num);
			recv_len = recv_packet(&packet, server_sk_num, &client);
			
			if (recv_len != CRC_ERROR) {
				/*fork will go here */
				if ((pid = fork()) < 0) {
					perror("fork");
					exit(-1);
				}
				//process child
				if (pid == 0) {
					process_client(server_sk_num, buf, recv_len, &client, &packet);
					exit(0);
				}
			}
			
			// check to see if any children quit
			while (waitpid(-1, &status, WNOHANG) > 0) {
				printf("processed wait\n");
			}
			//printf("after process wait... back to select\n");
		}
	}
}

void process_client(int32_t server_sk_num, uint8_t *buf, int32_t recv_len, Connection *client,
	Packet *packet) {
	STATE state = START;
	int32_t data_file = 0;
	int32_t buf_size = 0;
	int32_t seq_num = START_SEQ_NUM;
	Window window;
	
	printf("Processing Client...\n");
	
	while (state != DONE) {
		
		switch (state) {
			
			case START:
				state = FILENAME;
				break;
			
			case FILENAME:
				printf("\nSTATE: FILENAME\n");
				seq_num = 1;
				state = filename(client, packet, buf, recv_len, &data_file, &buf_size, &window);
				break;
				
			case SEND_DATA:
				printf("\nSTATE: SEND_DATA\n");
				state = send_data(client, packet, data_file, buf_size, &seq_num, &window);
				break;
			
			case PROCESS:
				printf("\nSTATE: PROCESS\n");
				state = process(client, &window);
				break;
			
			case WAIT_ON_ACK:
				printf("\nSTATE: WAIT_ON_ACK\n");
				state = wait_on_ack(client, &window);
				break;
			
			case DONE:
				printf("\nSTATE: DONE\n");
				destroy_window(&window);
				break;
			
			default:
				printf("In default and you should not be here!!!!\n");
				state = DONE;
				break;
		}
	}
}

STATE filename(Connection *client, Packet *packet, uint8_t *buf, int32_t recv_len, int32_t *data_file,
	int32_t *buf_size, Window *window) {
	int32_t window_size;
	uint8_t response[1];
	char fname[MAX_LEN];
	
	memcpy(buf_size, packet->payload, 4);
	memcpy(&(window_size), packet->payload + 4, 4);
	init_window(window, window_size);
	memcpy(fname, packet->payload + 8, recv_len - 8);
	
	/* Create client socket to allow for processing this particular client */
	
	if ((client->sk_num = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("filename, open client socket");
		exit(-1);
	}
	
	if (((*data_file) = open(fname, O_RDONLY)) < 0) {
		send_packet(response, 0, client, FNAME_BAD, 0);
		return DONE;
	}
	else {
		send_packet(response, 0, client, FNAME_OK, 0);
		return SEND_DATA;
	}
}

STATE send_data(Connection *client, Packet *packet, int32_t data_file, 
	int buf_size, int32_t *seq_num, Window *window) {
	
	int32_t len_read = 0;
	
	// Send a packet
	if (is_closed(window)) {
		return WAIT_ON_ACK;
	}
	else {
		len_read = read(data_file, packet->payload, buf_size);
		packet->seq_num = window->middle;
		packet->size = len_read + HEADER_LENGTH;
		
		switch (len_read) {
			case -1:
				perror("send_data, read error");
				return DONE;
				break;
			case 0:
				packet->flag = END_OF_FILE;
				construct(packet);
				send_packet2(packet, client);
				printf("File Transfer Complete.\n");
				return DONE;
				break;
			default:
				packet->flag = DATA;
				construct(packet);
				send_packet2(packet, client);
				window->middle++;
				add_to_buffer(window, packet);
				break;
		}	
	}
	
	// Check for RRs and SREJs
	if (select_call(client->sk_num, 0, 0, NOT_NULL) == 1)
		return PROCESS;
	
	return SEND_DATA;
}

STATE process(Connection *client, Window *window) {
	uint32_t crc_check = 0;
	Packet incomming;
	Packet resend;
	int32_t rr, srej;
	
	printf("Reading packet...\n");
	crc_check = recv_packet(&incomming, client->sk_num, client);
	
	printf("After recv_packet\n");
	if (crc_check == CRC_ERROR) {
		printf("CRC errror in packet %d\n", incomming.seq_num);
		return WAIT_ON_ACK;
	}
	
	if (incomming.flag == ACK) {
		rr = incomming.seq_num;
		slide(window, rr);
	}
	else if (incomming.flag == SREJ) {
		//rr = htonl(((int32_t *)incomming.payload)[0]);
		srej = incomming.seq_num;
		get_from_buffer(window, &resend, srej);
		send_packet2(&resend, client);
	}
	
	return SEND_DATA;
	
}

STATE wait_on_ack(Connection *client, Window *window) {
	static int32_t send_count = 0;
	Packet packet;
	
	send_count++;
	if (send_count > 10) {
		printf("Sent data 10 times, no ACK, client session terminated.\n");
		return DONE;
	}
	
	if (select_call(client->sk_num, 1, 0, NOT_NULL) != 1) {
		get_from_buffer(window, &packet, window->bottom);
	
		printf("No packets reseived, sending nudge\n");
		
		if (send_packet2(&packet, client) < 0) {
			perror("send packet, timeout_on_ack");
			exit(-1);
		}
	
		return WAIT_ON_ACK;
	}
	
	send_count = 0;
	return PROCESS;
}













