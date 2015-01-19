#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pcap.h>

#define UDP_MIN_SIZE 28
#define PORT_LENGTH 2

typedef struct udp_obj
{
	u_char *frame;
	int frame_length;
	
	u_int16_t opcode;
	u_int16_t dest;
   u_int16_t src;
} udp_obj;

int udp_init(udp_obj **to_init, u_char *packet, int size) {
	udp_obj *this;
	
	if (size < UDP_MIN_SIZE) {
		return -1;
	}
	
	this = calloc(1, sizeof(udp_obj));
	
	this->frame = calloc(size, sizeof(u_char));
	memcpy(this->frame, packet, size);
	
	this->opcode = ntohs(((u_int16_t *)packet)[2]);	
	this->dest = ntohs(((u_int16_t *)packet)[1]);	
	this->src = ntohs(((u_int16_t *)packet)[0]);
	
	*to_init = this;
	
	return 0;
}

void udp_print(udp_obj *this) {	
	int i;
	
	printf("      UDP Frame ");
	printf("  opcode: %d", this->opcode);
	printf(" | Destination PORT:  %d", this->dest);
	printf(" | Source PORT:  %d", this->src);	
	
	printf(" | Packet: ");
	for(i = 0; i < 8; i++) {
		printf("%02X", this->frame[i]);
	}
	
	printf("\n");
	
}

void udp_print2(udp_obj *this) {	
	
	printf("\n\tUDP Header");
	printf("\n\t\tSource Port:  %d", this->src);	
	printf("\n\t\tDest Port:  %d", this->dest);
	
	printf("\n");
	
}

void udp_free(udp_obj **this) {
	if (this && *this && (*this)->frame) {
		free((*this)->frame);
		free(*this);
		*this = NULL;
	}
}
