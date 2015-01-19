#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#define MAC_LENGTH 6
#define E_TYPE_BYTE 12
#define E_FRAME_SIZE 14
#define IP_TYPE_NO 1
#define ARP_TYPE_NO 2
#define UNKNOWN_TYPE_NO 0


typedef struct ethr_obj
{
	u_char *frame;
	int frame_length;
   u_char dest[6];
   u_char src[6];
   u_char type[2];
	u_char *data;
	int data_length;
} ethr_obj;

int ethr_init(ethr_obj **to_init, struct pcap_pkthdr *header, const u_char *packet) {
	ethr_obj *this;
	
	if (header->caplen < E_FRAME_SIZE) {
		return -1;
	}
	
	this = calloc(1, sizeof(ethr_obj));
	
	this->frame_length = header->caplen;
	this->frame = calloc(this->frame_length, sizeof(u_char));
	memcpy(this->frame, packet, this->frame_length);

	memcpy(this->dest, this->frame, MAC_LENGTH);

	memcpy(this->src, this->frame + MAC_LENGTH, MAC_LENGTH);
	
	memcpy(this->type, this->frame + E_TYPE_BYTE, 2);
	
	this->data_length = this->frame_length - E_FRAME_SIZE;
	this->data = calloc(this->data_length, sizeof(u_char));
	memcpy(this->data, this->frame + E_FRAME_SIZE, this->data_length);
	
	*to_init = this;
	
	return 0;
}

void ethr_print(ethr_obj *this) {
	int i;
	
	printf("Ethernet Frame ");
	printf("   Destination MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%02X", this->dest[i]);
	}
	
	printf(" | Source MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%02X", this->src[i]);
	}
	
	printf(" | Type: %02x\n", this->type[1]);
	
}

u_char *ethr_data(ethr_obj *this) {
	u_char *data;
	
	data = calloc(this->data_length, sizeof(u_char));
	memcpy(data, this->data, this->data_length);
	return data;
}

int ethr_data_length(ethr_obj *this) {
	return this->data_length;
}

int ethr_type(ethr_obj *this) {
	if (this->type[0] == 0x08) {
		if (this->type[1] == 0)
			return IP_TYPE_NO;
		else if (this->type[1] == 6)
			return ARP_TYPE_NO;
	}
	return UNKNOWN_TYPE_NO;
}

void ethr_free(ethr_obj **this) {
	if (this != NULL) {
		free((*this)->frame);
		free((*this)->data);
		free(*this);
		*this = NULL;
	}
}