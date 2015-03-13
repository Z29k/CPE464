/* rcopy
	To run: rcopy fromFile toFile bufferSize errorRate serverMachine serverPort
	
	By: Hugh Smith */
	
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
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
	DONE, FILENAME, RECV_DATA, FILE_OK, MISSING
};

STATE filename(char * fname, int32_t buf_size, Window *window);
STATE recv_data(Window *window, int32_t output_file);
STATE missing(Window *window, int32_t output_file);
void check_args(int argc, char **argv);

Connection server;

int main(int argc, char **argv) {
	int32_t output_file = 0;
	int32_t select_count = 0;
	Window window;
	STATE state = FILENAME;
	
	check_args(argc, argv);
	
	sendtoErr_init(atof(argv[4]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
	
	init_window(&window, atoi(argv[5]));
	
	state = FILENAME;
	
	while (state != DONE) {
		
		switch (state) {
			case FILENAME:
				printf("\nSTATE: FILENAME\n");
				/* Every time we try to start/restart a connection get a new socket */				
				if (udp_client_setup(argv[6], atoi(argv[7]), &server) < 0)
					exit(-1);
				
				state = filename(argv[1], atoi(argv[3]), &window);
				
				/*if no response from server then repeat sending filename (close socket_ so you can open another */
				if (state == FILENAME)
					close(server.sk_num);
					
				select_count++;
				if (select_count > 9) {
					printf("Server unreachable, client terminating\n");
					state = DONE;
				}
				break;
				
			case FILE_OK:
				printf("\nSTATE: FILE_OK\n");
				select_count = 0;
				
				if ((output_file = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0600)) < 0) {
					perror("File open error");
					state = DONE;
				}
				else {
					state = RECV_DATA;
				}
				break;
				
			case RECV_DATA:
				printf("\nSTATE: RECV_DATA\n");
				state = recv_data(&window, output_file);
				break;
				
				
			case MISSING:
				printf("\nSTATE: MISSING\n");
				state = missing(&window, output_file);
				break;
				
			case DONE:
				printf("\nSTATE: DONE\n");
				destroy_window(&window);
				break;
			
			default:
				printf("ERROR - in default state\n");
				break;
		}
	}
	return 0;
}
	
STATE filename(char *fname, int32_t buf_size, Window *window) {
	uint8_t buf[MAX_LEN];
	int32_t fname_len = strlen(fname) + 1;
	int32_t recv_check = 0;
	Packet packet;
	memcpy(buf, &buf_size, 4);
	memcpy(buf + 4, &(window->size), 4);
	memcpy(&buf[8], fname, fname_len);
	
	send_packet(buf, fname_len + 8, &server, FNAME, 0);
	
	if (select_call(server.sk_num, 1, 0, NOT_NULL) == 1) {
		recv_check = recv_packet(&packet, server.sk_num, &server);
		/*check for bit flip ... if so send the file name again */

		if (recv_check == CRC_ERROR)
			return FILENAME;

		if (packet.flag == FNAME_BAD) {
			printf("File %s not found\n", fname);
			return (DONE);
		}

		return (FILE_OK);
	}
	
	return FILENAME;
}

STATE recv_data(Window *window, int32_t output_file) {
	int32_t data_len = 0;
	Packet data_packet;
	Packet ack;
	
	if (select_call(server.sk_num, 10, 0, NOT_NULL) == 0) {
		printf("Timeout after 10 seconds, client done.\n");
		return DONE;
	}
	
	data_len = recv_packet(&data_packet, server.sk_num, &server);
	
	if (data_len == CRC_ERROR) 
		return MISSING;
		
	/* send ACK */
	ack.seq_num = data_packet.seq_num + 1;
	ack.flag = ACK;
	ack.size = HEADER_LENGTH;
	construct(&ack);
	send_packet2(&ack, &server);
	printf("Sendding RR%d\n", ack.seq_num);
	
	if (data_packet.flag == END_OF_FILE) {
		close(output_file);
		printf("file done\n");
		return DONE;
	}
	
	if (data_packet.seq_num == window->bottom) {
		slide(window, window->bottom + 1);
		write(output_file, data_packet.payload, data_len);
	}
	else if (data_packet.seq_num < window->bottom 
		&& data_packet.seq_num > window->top) {
		printf("Received out of window packet. Quitting...\n");
		return DONE;
	}
	else {
		add_to_buffer(window, &data_packet);
		window->middle = window->bottom;
		return MISSING;
	}
	
	return RECV_DATA;	
}

STATE missing(Window *window, int32_t output_file) {
	int i;
	int32_t data_len = 0;
	Packet srej;
	Packet rr;
	Packet data_packet;
	Packet to_write;
	
	
	/* send SREJ */
	srej.seq_num = window->middle;
	srej.flag = SREJ;
	srej.size = HEADER_LENGTH;
	construct(&srej);
	send_packet2(&srej, &server);
	printf("Sendding SREJ%d\n", srej.seq_num);
	
	if (select_call(server.sk_num, 10, 0, NOT_NULL) == 0) {
		printf("Timeout after 10 seconds, client done.\n");
		return DONE;
	}
	
	
	data_len = recv_packet(&data_packet, server.sk_num, &server);
	
	if (data_len == CRC_ERROR) 
		return MISSING;
	
	if (data_packet.flag == END_OF_FILE) {
		if (window->bottom == data_packet.flag) {
			printf("file done\n");
			return DONE;
		}
		return MISSING;
	}
	
	if (is_in_window(window, data_packet.seq_num)) {
		add_to_buffer(window, &data_packet);
		
		for (i = window->bottom; i <= window->top + 1; i++) {
			window->middle = i;
			if (is_valid(window, i) == 0) {
				break;
			}
		}
		
		for (i = window->bottom; i < window->middle; i++) {
			remove_from_buffer(window, &to_write, i);
			write(output_file, to_write.payload, data_len);			
		}
		
		// send RR
		rr.seq_num = window->middle;
		rr.flag = ACK;
		rr.size = HEADER_LENGTH;
		construct(&rr);
		send_packet2(&rr, &server);
		printf("Sendding RR%d\n", rr.seq_num);
		
		// All packets accounted for
		if (is_closed(window)) {
			slide(window, window->middle);
			return RECV_DATA;		
		}
		
		slide(window, window->middle);
		
		return MISSING;
	}
	
	printf("Recieved out of window packet, exiting...\n");
	return DONE;
}

void check_args(int argc, char **argv) {
	if (argc != 8) {
		printf("Usage %s fromFile toFile buffer_size error_rate window_size hostname port\n", argv[0]);
		exit(-1);
	}
	
	if (strlen(argv[1]) > 1000) {
		printf("FROM filename to long, needs to be less than 1000 and is %d\n", (int)strlen(argv[1]));
		exit(-1);
	}
	
	if (strlen(argv[2]) > 1000) {
		printf("TO filename to long, needs to be less than 1000 and is %d\n", (int)strlen(argv[2]));
		exit(-1);
	}
	
	if (atoi(argv[3]) < 400 || atoi(argv[3]) > 1400) {
		printf("Buffer size needs to be between 400 and 1400 and is %d\n", atoi(argv[3]));
		exit(-1);
	}
	
	if (atoi(argv[4]) < 0 || atoi(argv[4]) >= 1) {
		printf("Error rate needs to be between 0 and less then 1 and is %d\n", atoi(argv[4]));
		exit(-1);
	}
}
