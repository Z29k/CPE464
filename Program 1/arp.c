#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pcap.h>

#define ARP_MIN_SIZE 28
#define IP_LENGTH 4
#define MAC_LENGTH 6
#define REQUEST 1
#define REPLY 2

typedef struct arp_obj
{
	u_char *frame;
	int frame_length;
	
	//u_char opcode[2];
   u_int16_t opcode;
	u_char dest_ip[IP_LENGTH];
   u_char dest_mac[MAC_LENGTH];
   u_char src_ip[IP_LENGTH];
   u_char src_mac[MAC_LENGTH];
} arp_obj;

int arp_init(arp_obj **to_init, u_char *packet, int size) {
	arp_obj *this;
	
	if (size < ARP_MIN_SIZE) {
		return -1;
	}
	
	this = calloc(1, sizeof(arp_obj));
	
	this->frame_length = size;
	this->frame = calloc(this->frame_length, sizeof(u_char));
	memcpy(this->frame, packet, this->frame_length);
	
	this->opcode = this->frame[6] << 8 | this->frame[7];
	//this->opcode = ntohs(this->opcode);
	memcpy(this->dest_mac, this->frame + 18, MAC_LENGTH);
	memcpy(this->dest_ip, this->frame + 24, IP_LENGTH);
	memcpy(this->src_mac, this->frame + 8, MAC_LENGTH);
	memcpy(this->src_ip, this->frame + 14, IP_LENGTH);
	
	*to_init = this;
	
	return 0;
}

void arp_print2(arp_obj *this) {
	int i;
	
	printf("\n\tARP header ");
	printf("\n\t\tOpcode: ");
	
	if (this->opcode == REQUEST)
		printf("Request");
	else if (this->opcode == REPLY)
		printf("Reply");

	
	printf("\n\t\tSender MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%x", (int)(this->src_mac[i]));
	}
	
	printf("\n\t\tSender IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%d", (int)(this->src_ip[i]));
	}
	
	printf("\n\t\tTarget MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%x", (int)(this->dest_mac[i]));
	}
	
	printf("\n\t\tTarget IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%d", (int)(this->dest_ip[i]));
	}
	
	printf("\n");
	
}

void arp_print(arp_obj *this) {
	int i;
	
	printf("   ARP header");
	printf("    Opcode: %u", this->opcode);
	
	printf(" | Destination IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%03d", (int)(this->dest_ip[i]));
	}
	
	printf(" | Source IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%03d", (int)(this->src_ip[i]));
	}
	
	printf(" | Destination MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%02X", (int)(this->dest_mac[i]));
	}
	
	printf(" | Source MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%02X", (int)(this->src_mac[i]));
	}
	
	printf("\n");
	
}

void arp_free(arp_obj **this) {
	if (this && *this && (*this)->frame) {
		free((*this)->frame);
		free(*this);
		*this = NULL;
	}
}
