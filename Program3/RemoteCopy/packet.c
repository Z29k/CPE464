#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "packet.h"
#include "cpe464.h"
#include "networks.h"


void construct(Packet *packet) {
	uint8_t *data = packet->raw;
	int i;
	
	for (i = 0; i < MAX_PACKET_LENGTH + HEADER_LENGTH; i++)
		data[i] = 0;
	
	((uint32_t *)data)[0] = htonl(packet->seq_num);
	
	data[6] = packet->flag;
	*((uint32_t *)(data+7)) = htonl(packet->size);
	memcpy(data+11, &packet->payload, MAX_PACKET_LENGTH);
	
	packet->checksum = in_cksum((uint16_t *) data, packet->size);
	
	((int16_t *)data)[2] = packet->checksum;
	
	
	//printf("Constructing packet...\n");
	//print_packet(packet);
	
}

int deconstruct(Packet *packet) {
	uint8_t *data = packet->raw;
	int i;
	
	for (i = 0; i < MAX_PACKET_LENGTH; i++) 
		packet->payload[i] = 0;
	
	packet->seq_num = ntohl(((uint32_t *)data)[0]);
	
	packet->checksum = ntohs(*((int16_t *)(data + 4)));
	packet->flag = data[6];
	packet->size = ntohl(*((uint32_t *)(data+7)));
	
	memcpy(packet->payload, data + HEADER_LENGTH, packet->size - HEADER_LENGTH);
	
	//printf("Deconstructing packet...\n");
	//print_packet(packet);
	
	return in_cksum((uint16_t *) data, packet->size);
}

void print_packet(Packet *packet) {
	int i;
	
	printf("Packet:\n");
	printf("   Seq#: %d, Flag: %d, Checksum: %d, Size %d\n", packet->seq_num,
		packet->flag, packet->checksum, packet->size);
	printf("   Data: ");
	
	for (i = 0; i < packet->size - HEADER_LENGTH; i++) 
		printf("%c", packet->payload[i]);
		
	printf("\n\n");	
}	
