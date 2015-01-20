/** Trace
 *  Program 1 for CPE 464
 *  Cal Poly
 *  Author: Cody Jones
 */

#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>

#include "ethernet.h"
#include "ip.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"

#define ETHR 0
#define IP 1
#define ARP 2
#define UDP 3
#define TCP 4 
#define ICMP 5
#define UNKNOWN 6
#define PAYLOAD 7

int ethr(struct pcap_pkthdr *, const u_char *, u_char **, int *);
int ip(u_char **, int *, u_char **);
int arp(u_char **, int *);
int udp(u_char **, int *);
int tcp(u_char **, int *, u_char **);
int icmp(u_char **);

int main(int argc, char **argv) {
	pcap_t *pcap;
	struct pcap_pkthdr *header;
	const u_char *packet;
	u_char *pdu;
	u_char *psuedo_header;
	int pdu_length;
	int next_type;
	int cur;
	char errorBuffer[PCAP_ERRBUF_SIZE];
	
	if (argc < 2) {
		printf("No file given\n");
		return 0;
	}
	
	// Open Pcap file
	pcap = pcap_open_offline(argv[1], errorBuffer);
	
	if (!pcap) {
		printf("%s\n", errorBuffer);
		return 1;
	}
	
	cur = 1;
	
	// Open each Packet
	while (pcap_next_ex(pcap, &header, &packet) == 1) {
		printf("\nPacket number: %d  Packet Len: %d\n", cur, header->caplen);
		
		// Read and remove ethernet header
		next_type = ethr(header, packet, &pdu, &pdu_length);
		
		// Continue reading and removing headers until all that remains is a payload.
		while (next_type != UNKNOWN && next_type != PAYLOAD) {
			switch (next_type) {
				case IP:
					next_type = ip(&pdu, &pdu_length, &psuedo_header);
					break;
				case ARP:
					next_type = arp(&pdu, &pdu_length);
					break;
				case UDP:
					next_type = udp(&pdu, &pdu_length);
					break;
				case TCP:
					next_type = tcp(&pdu, &pdu_length, &psuedo_header);
					break;
				case ICMP:
					next_type = icmp(&pdu);
					break;
			}
		}
		if (next_type == UNKNOWN)
			printf("Unknown PDU");
		printf("\n");
		cur++;
	}
	
	return 0;
}

int ethr(struct pcap_pkthdr *header, const u_char *packet, u_char **pdu, int *pdu_length)  {
	int next_type;
	struct ethr_obj *ethr;
	
	next_type = UNKNOWN;
	
	ethr_init(&ethr, header, packet);
	ethr_print2(ethr);
	*pdu = ethr_data(ethr);
	*pdu_length = ethr_data_length(ethr);
	
	if (ethr_type(ethr) == IP_TYPE_NO)
		next_type = IP;
	else if (ethr_type(ethr) == ARP_TYPE_NO)
		next_type = ARP;
	
	ethr_free(&ethr);
	
	return next_type;
}

int ip(u_char **pdu, int *pdu_length, u_char **psuedo_header) {
	struct ip_obj *ip;
	int next_type;
	
	next_type = PAYLOAD;
	
	ip_init(&ip, *pdu, *pdu_length);
	ip_print2(ip);
	
	
	free(*pdu);
	*pdu = ip_data(ip);
	*pdu_length = ip_data_length(ip);
	*psuedo_header = ip_psuedo_header(ip);
	
	if (ip_protocol(ip) == ICMP_PROTOCOL)
		next_type = ICMP;
	else if (ip_protocol(ip) == TCP_PROTOCOL)
		next_type = TCP;
	else if (ip_protocol(ip) == UDP_PROTOCOL)
		next_type = UDP;
	
	ip_free(&ip);
	
	return next_type;
}

int arp(u_char **pdu, int *pdu_length) {
	struct arp_obj *arp;
	
	arp_init(&arp, *pdu, *pdu_length);
	arp_print2(arp);
	free(*pdu);
	
	arp_free(&arp);
	
	return PAYLOAD;
}

int udp(u_char **pdu, int *pdu_length) {
	struct udp_obj *udp;
	
	udp_init(&udp, *pdu, *pdu_length);
	udp_print2(udp);
	free(*pdu);
	
	udp_free(&udp);
	
	return PAYLOAD;
}

int tcp(u_char **pdu, int *pdu_length, u_char **psuedo_header) {
	struct tcp_obj *tcp;
	
	tcp_init(&tcp, *pdu, *psuedo_header, *pdu_length);
	tcp_print2(tcp);
	free(*pdu);
	free(*psuedo_header);
	
	tcp_free(&tcp);
	
	return PAYLOAD;
}

int icmp(u_char **pdu) {
	printf("\n\tICMP Header");
	if (icmp_type(*pdu) == ECHO_REQUEST)
		printf("\n\t\tType: Request\n" );
	else if (icmp_type(*pdu) == ECHO_REPLY)
		printf("\n\t\tType: Reply\n" );
	else
		printf("\n\t\tType: Unknown\n" );
	
	free(*pdu);
	
	return PAYLOAD;
}