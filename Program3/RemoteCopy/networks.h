/* used for network functions 
	Written By: Hugh Smith
	Used By:  Cody Jones
	Cal Poly CPE 464 Winter 2015
*/

#ifndef _NETWORKS_H_
#define _NETWORKS_H_

#define MAX_LEN 1500
#define START_SEQ_NUM 1

#include "packet.h"

enum FLAG {
	FNAME, DATA, FNAME_OK, FNAME_BAD, ACK, SREJ, END_OF_FILE, CRC_ERROR = -1
};

enum SELECT {
	SET_NULL, NOT_NULL
};

typedef struct connection Connection;

struct connection {
	int32_t sk_num;
	struct sockaddr_in remote;
	uint32_t len;
};

int32_t udp_server();

int32_t udp_client_setup(char* hostname, uint16_t port_num, Connection *connection);

int32_t select_call(int32_t socket_num, int32_t seconds, int32_t microseconds, int32_t set_null);

int32_t send_packet(uint8_t *data, uint32_t len, Connection *connection, uint8_t flag, 
	uint32_t seq_num);
	
int32_t send_packet2(Packet *packet, Connection *connection);
	
int32_t recv_packet(Packet *packet, int32_t recv_sk_num, Connection *connection);
	
int32_t send_buf(uint8_t *buf, uint32_t len, Connection *connection, uint8_t flag, 
	uint32_t seq_num, uint8_t *packet);

int32_t recv_buf(uint8_t *buf, int32_t len, int32_t recv_sk_num, Connection *from_connection,
	uint8_t *flag, int32_t *seq_num);

#endif
