#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#define MAC_LENGTH 6
#define E_TYPE_BYTE 12
#define E_FRAME_SIZE 14
#define IP_TYPE_NO 0x0800
#define ARP_TYPE_NO 0x0806
#define UNKNOWN_TYPE_NO 0


typedef struct ethr_obj
{
	u_char *frame;
	int frame_length;
   u_char dest[6];
   u_char src[6];
   u_int16_t type;
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
	
	this->type = ntohs(((u_int16_t *)this->frame)[6]);
	
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
	
	printf(" | Type: %02x\n", this->type);
	
}

void ethr_print2(ethr_obj *this) {
	int i;
	
	printf("\n\tEthernet Header\n");
	printf("\t\tDest MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%x", this->dest[i]);
	}
	
	printf("\n\t\tSource MAC: ");
	
	for (i = 0; i < MAC_LENGTH; i++) {
		if (i)
			printf(":");
		printf("%x", this->src[i]);
	}
	
	printf("\n\t\tType: ");
	if (this->type == IP_TYPE_NO)
		printf("IP");
	else if (this->type == ARP_TYPE_NO)
		printf("ARP");
	else 
		printf("Unknown");
	printf("\n");
	
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
	return this->type;
}

void ethr_free(ethr_obj **this) {
	if (this != NULL) {
		free((*this)->frame);
		free((*this)->data);
		free(*this);
		*this = NULL;
	}
}