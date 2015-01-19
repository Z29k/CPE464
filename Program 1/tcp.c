#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pcap.h>

#include "checksum.h"

#define SYN_MASK 0x2
#define FIN_MASK 0x1
#define RST_MASK 0x0

typedef struct tcp_obj
{
	u_char *frame;
	int frame_length;
	
	u_int16_t src;
	u_int16_t dest;
	u_int32_t seq;
	u_int32_t ack;
	u_int16_t window;
	u_int16_t checksum;
	u_char psuedo[12];
	u_char flags;
	
} tcp_obj;

void tcp_checksum(tcp_obj *);

int tcp_init(tcp_obj **to_init, u_char *packet, u_char *psuedo, int size) {
	tcp_obj *this;
	
	this = calloc(1, sizeof(tcp_obj));
	
	this->frame_length = size;
	this->frame = calloc(this->frame_length, sizeof(u_char));
	memcpy(this->frame, packet, this->frame_length);
	memcpy(this->psuedo, psuedo, 12);
	this->psuedo[11] = size & 0x00FF;
	
	this->src = ntohs(*((u_int16_t *)this->frame));
	this->dest = ntohs(((u_int16_t *)this->frame)[1]);
	this->seq = ntohl(((u_int32_t *)this->frame)[1]);
	this->ack = ntohl(((u_int32_t *)this->frame)[2]);
	this->window = ntohs(((u_int16_t *)this->frame)[7]);
	this->flags = this->frame[13];
	
	tcp_checksum(this);
	
	*to_init = this;
	
	return 0;
}

void tcp_print(tcp_obj *this) {
	int i;
	
	printf("      TCP Frame ");
	printf("  Source Port: %u", this->src);
	printf(" | Destination Port: %u", this->dest);
	printf(" | Sequence Number: %u", this->seq);
	printf(" | Acknowledgment Number: %u", this->ack);
	printf(" | Window Size: %u", this->window);
	
	if (this->checksum)
		printf(" | Checksum: %d", this->checksum);
	else
		printf(" | Checksum: Good");
	
	this->flags & FIN_MASK ? printf(" | FIN : ON ") : printf(" | FIN : OFF");
	this->flags & SYN_MASK ? printf(" | SYN : ON ") : printf(" | SYN : OFF");
	this->flags & RST_MASK ? printf(" | RST : ON ") : printf(" | RST : OFF");
	
	/*
	printf(" | Payload: ");
	for(i = 0; i < this->frame_length; i++) {
		printf("%02X", this->frame[i]);
	}
	*/
	printf(" | Psuedo Header: ");
	for(i = 0; i < 12; i++) {
		printf("%02X", this->psuedo[i]);
	}
	
	
	printf("\n");
	
}

void tcp_checksum(tcp_obj *this) {
	u_char  *to_check;
	
	to_check = calloc(this->frame_length + 12, sizeof(u_char));
	memcpy(to_check, this->psuedo, 12);
	memcpy(to_check + 12, this->frame, this->frame_length);
	
	this->checksum = in_cksum((u_int16_t *)to_check, this->frame_length + 12);
}

void tcp_free(tcp_obj **this) {
	if (this && *this && (*this)->frame) {
		free((*this)->frame);
		free(*this);
		*this = NULL;
	}
}
