#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pcap.h>

#include "checksum.h"

#define IP_MIN_SIZE 20
#define IP_LENGTH 4
#define WORD_LENGTH 4
#define ICMP_PROTOCOL 1
#define UDP_PROTOCOL 17
#define TCP_PROTOCOL 6

typedef struct ip_obj
{
	u_char *frame;
	int frame_length;
	
	u_char tos;
	u_char ttl;
	u_char ihl;
	u_char protocol;
	u_int16_t length;
	u_int16_t checksum;
   u_char dest[4];
   u_char src[4];
	
	u_char *data;
	int data_length;
} ip_obj;


int ip_init(ip_obj **to_init, u_char *packet, int size) {
	ip_obj *this;
	
	if (size < IP_MIN_SIZE) {
		return -1;
	}
	
	this = calloc(1, sizeof(ip_obj));
	
	this->frame_length = size;
	this->frame = calloc(this->frame_length, sizeof(u_char));
	memcpy(this->frame, packet, this->frame_length);
	
	this->tos = this->frame[1];
	this->ttl = this->frame[8];
	this->protocol = this->frame[9];
	this->ihl = WORD_LENGTH * (this->frame[0] & 0x0F);
	this->length = ntohs(((u_int16_t *)packet)[1]);
	
	this->checksum = in_cksum((u_int16_t *)this->frame, this->ihl);
	
	memcpy(this->dest, this->frame + 16, 4);
	memcpy(this->src, this->frame + 12, 4);
	
	this->data_length = this->length - this->ihl;
	this->data = calloc(this->data_length, sizeof(u_char));
	memcpy(this->data, this->frame + this->ihl, this->data_length);
	
	*to_init = this;
	
	return 0;
}

void ip_print(ip_obj *this) {
	int i;
	
	printf("   IP Frame ");
	printf("      TOS: %02X", this->tos);
	printf(" | Time to Live: %02d", (int)(this->ttl));
	
	if (this->checksum)
		printf(" | Checksum: %d", this->checksum);
	else
		printf(" | Checksum: GOOD");
	
	printf(" | Destination IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%03d", (int)(this->dest[i]));
	}
	
	printf(" | Source IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%03d", (int)(this->src[i]));
	}

	printf(" | IHL: %d", this->ihl);
	printf(" | Length: %d", this->length);
	printf(" | Data Length: %d", this->data_length);
	
	/*
	printf(" | Packet: ");
	for(i = 0; i < this->frame_length; i++) {
		printf("%02X", this->frame[i]);
	}
	*/
	printf("\n");
	
}

void ip_print2(ip_obj *this) {
	int i;
	u_int16_t read_checksum;
	
	printf("\n\tIP Header ");
	printf("\n\t\tTOS: 0x%x", this->tos);
	printf("\n\t\tTTL : %d", (int)(this->ttl));
	
	printf("\n\t\tProtocol: ");
	
	if (this->protocol == ICMP_PROTOCOL)
		printf("ICMP");
	else if (this->protocol == UDP_PROTOCOL)
		printf("UDP");
	else if (this->protocol == TCP_PROTOCOL)
		printf("TCP");
	else
		printf("Unknown");
	
	read_checksum = ntohs(((u_int16_t *)this->frame)[5]);
	if (this->checksum)
		printf("\n\t\tChecksum: Incorrect (0x%x)", read_checksum);
	else
		printf("\n\t\tChecksum: Correct (0x%x)", read_checksum);
	
	printf("\n\t\tSender IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%d", (int)(this->src[i]));
	}
	
	printf("\n\t\tDest IP: ");
	
	for (i = 0; i < IP_LENGTH; i++) {
		if (i)
			printf(".");
		printf("%d", (int)(this->dest[i]));
	}
	printf("\n");
	
}

u_char *ip_data(ip_obj *this) {
	u_char *data;
	
	data = calloc(this->data_length, sizeof(u_char));
	memcpy(data, this->data, this->data_length);
	return data;
}

int ip_data_length(struct ip_obj *this) {
	return this->data_length;
}

int ip_protocol(struct ip_obj *this) {
	return this->protocol;
}

u_char *ip_psuedo_header(struct ip_obj *this) {
	u_char *psuedo;
	
	psuedo = calloc(12, sizeof(u_char));
	memcpy(psuedo, this->src, IP_LENGTH);
	memcpy(psuedo + IP_LENGTH, this->dest, IP_LENGTH);
	psuedo[9] = this->protocol;
	psuedo[10] = (this->data_length & 0xFF00) >> 8;
	psuedo[11] = this->data_length & 0x00FF;
	
	return psuedo;
}

void ip_free(ip_obj **this) {
	if (this && *this && (*this)->frame) {
		free((*this)->frame);
		free((*this)->data);
		free(*this);
		*this = NULL;
	}
}
