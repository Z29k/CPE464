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

typedef enum State STATE;

enum State {
	DONE, FILENAME, RECV_DATA, FILE_OK
};

STATE filename(char * fname, int32_t buf_size);
STATE recv_data(int32_t output_file);
void check_args(int argc, char **argv);

Connection server;

int main(int argc, char **argv) {
	int32_t output_file = 0;
	int32_t select_count = 0;
	STATE state = FILENAME;
	
	check_args(argc, argv);
	
	sendtoErr_init(atof(argv[4]), DROP_ON, FLIP_ON, DEBUG_ON, RSEED_ON);
	
	state = FILENAME;
	
	while (state != DONE) {
		
		switch (state) {
			case FILENAME:
				/* Every time we try to start/restart a connection get a new socket */				
				if (udp_client_setup(argv[5], atoi(argv[6]), &server) < 0)
					exit(-1);
				
				state = filename(argv[1], atoi(argv[3]));
				
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
				state = recv_data(output_file);
				break;
				
			case DONE:
				break;
			
			default:
				printf("ERROR - in default state\n");
				break;
		}
	}
	return 0;
}
	
STATE filename(char *fname, int32_t buf_size) {
	uint8_t packet[MAX_LEN];
	uint8_t buf[MAX_LEN];
	uint8_t flag = 0;
	int32_t seq_num = 0;
	int32_t fname_len = strlen(fname) + 1;
	int32_t recv_check = 0;
	
	memcpy(buf, &buf_size, 4);
	memcpy(&buf[4], fname, fname_len);
	
	send_buf(buf, fname_len + 4, &server, FNAME, 0, packet);
	
	if (select_call(server.sk_num, 1, 0, NOT_NULL) == 1) {
	
		recv_check = recv_buf(packet, 1000, server.sk_num, &server, &flag, &seq_num);

		/*check for bit flip ... if so send the file name again */

		if (recv_check == CRC_ERROR)
			return FILENAME;

		if (flag == FNAME_BAD) {
			printf("File %s not found\n", fname);
			return (DONE);
		}

		return (FILE_OK);
	}
	
	return FILENAME;
}

STATE recv_data(int32_t output_file) {
	int32_t seq_num = 0;
	uint8_t flag = 0;
	int32_t data_len = 0;
	uint8_t data_buf[MAX_LEN];
	uint8_t packet[MAX_LEN];
	static int32_t expected_seq_num = START_SEQ_NUM;
	
	if (select_call(server.sk_num, 10, 0, NOT_NULL) == 0) {
		printf("Timeout after 10 seconds, client done.\n");
		return DONE;
	}
	
	data_len = recv_buf(data_buf, 1400, server.sk_num, &server, &flag, &seq_num);
	
	/* do state RECV_DATA again if there is a crc error (don't send ack, don't write data) */
	if (data_len == CRC_ERROR) 
		return RECV_DATA;
		
	/* send ACK */
	send_buf(packet, 1, &server, ACK, 0, packet);
	
	if (flag == END_OF_FILE) {
		printf("file done\n");
		return DONE;
	}
	
	if (seq_num == expected_seq_num) {
		expected_seq_num++;
		write(output_file, &data_buf, data_len);
	}
	
	return RECV_DATA;	
}

void check_args(int argc, char **argv) {
	if (argc != 7) {
		printf("Usage %s fromFile toFile buffer_size error_rate hostname port\n", argv[0]);
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
