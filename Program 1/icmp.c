#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pcap.h>

int icmp_type(u_char *packet) {
	return (int)(*packet);
}