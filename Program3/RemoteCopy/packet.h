#ifndef PACKET_H
#define PACKET_H

#define HEADER_LENGTH 11

#define MAX_PACKET_LENGTH 1400

#include <sys/types.h>
#include <stdint.h>

typedef struct packet Packet;

struct packet {
	uint32_t seq_num;
	int16_t checksum;
	uint8_t flag;
	uint32_t size;
	uint8_t payload[MAX_PACKET_LENGTH];
	uint8_t raw[MAX_PACKET_LENGTH + HEADER_LENGTH];
};

void construct(Packet *packet);

int deconstruct(Packet *packet);

void print_packet(Packet *packet);

#endif
