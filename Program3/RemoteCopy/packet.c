#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "packet.h"
#include "cpe464.h"
#include "networks.h"


void construct(Packet *packet) {
	uint8_t *data = packet->raw;
	int16_t checksum;
	int i;
	
	for (i = 0; i < MAX_LEN; i++)
		data[i] = 0;
	
	((uint32_t *)data)[0] = htonl(packet->seq_num);
	data[6] = packet->flag;
	*((uint32_t *)(data+7)) = htonl(packet->size);
	memcpy(data+11, &packet->payload, MAX_LEN - HEADER_LENGTH);
	
	checksum = in_cksum((unsigned short *) data, MAX_LEN);
	
	*((uint16_t *)(data + 4)) = htons(checksum);
}

int deconstruct(Packet *packet) {
	uint8_t *data = packet->raw;
	int i;
	
	for (i = 0; i < MAX_LEN - HEADER_LENGTH; i++) 
		packet->payload[i] = 0;
		
	packet->seq_num = ntohl(*((uint32_t *)data));
	
	packet->checksum = ntohs(*((int16_t *)(data + 4)));
	packet->flag = data[6];
	packet->size = ntohl(*((uint32_t *)(data+7)));
	
	memcpy(packet->payload, data + HEADER_LENGTH, MAX_LEN - HEADER_LENGTH);
	
	return in_cksum((unsigned short *) data, MAX_LEN);
}

void print_packet(Packet *packet) {
	int i;
	
	printf("Packet:\n");
	printf("   Seq#: %d, Flag: %d, Checksum: %d, Size %d\n", packet->seq_num,
		packet->flag, packet->checksum, packet->size);
	printf("   Data: ");
	
	for (i = 0; i < packet->size; i++) 
		printf("%c", packet->payload[i]);
		
	printf("\n\n");	
}	
