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


void printPacket(struct pcap_pkthdr *, const u_char *);

int main(int argc, char **argv) {
	pcap_t *pcap;
	struct pcap_pkthdr *header;
	const u_char *packet;
	struct ethr_obj *ethr;
	struct ip_obj *ip;
	struct arp_obj *arp;
	struct udp_obj *udp;
	struct tcp_obj *tcp;
	int cur;
	
	char errorBuffer[PCAP_ERRBUF_SIZE];
	
	printf("Hello\n");
	
	if (argc == 1) {
		printf("No file given\n");
		return 0;
	}
	
	// Open Pcap file
	printf("Loading %s\n", argv[1]);
	pcap = pcap_open_offline(argv[1], errorBuffer);
	
	if (!pcap) {
		printf("%s\n", errorBuffer);
		return 1;
	}
	
	cur = 1;
	while (pcap_next_ex(pcap, &header, &packet) == 1) {
		printf("Packet %d\n", cur);		
		ethr_init(&ethr, header, packet);
		ethr_print(ethr);		
		
		if (ethr_type(ethr) == IP_TYPE_NO) {
			ip_init(&ip, ethr_data(ethr), ethr_data_length(ethr));
			ip_print(ip);
			
			if (ip_protocol(ip) == ICMP_PROTOCOL) {
				
				if (icmp(ip_data(ip)) == ECHO_REQUEST)
					printf("      ICMP:       TYPE: ECHO_REQUEST\n" );
				else if (icmp(ip_data(ip)) == ECHO_REPLY)
					printf("      ICMP:       TYPE: ECHO_REPLY\n" );
				else
					printf("      ICMP:       Type: UNKNOWN");
			}
			else if (ip_protocol(ip) == TCP_PROTOCOL) {
				tcp_init(&tcp, ip_data(ip), ip_psuedo_header(ip), ip_data_length(ip));
				tcp_print(tcp);
			}
			else if (ip_protocol(ip) == UDP_PROTOCOL) {
				udp_init(&udp, ip_data(ip), ip_data_length(ip));
				udp_print(udp);
			}
			else
				printf("Unknown protocol");
		}
		else if(ethr_type(ethr) == ARP_TYPE_NO) {
			arp_init(&arp, ethr_data(ethr), ethr_data_length(ethr));
			arp_print(arp);		
		}
		else
			printf("Unknown type\n");

		ethr_free(&ethr);
		printf("\n");
		cur++;
	}
	
	
	
	return 0;
}

void printPacket(struct pcap_pkthdr *header, const u_char *packet) {
	int i, count;
	printf("Packet\n");
	
	for (i = 0, count = 0; i < header->caplen; i++, count++) {
		printf("%02X ", packet[i]);
		
		if (count == 16)
			count = 0;
		
		if (count == 7)
			printf(" ");
		
		if (count == 15)
			printf("\n");
	}
	printf("\n");
}
