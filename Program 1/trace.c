#include <stdio.h>
#include <pcap.h>

int main(int argc, char **argv) {
	pcap_t *pcap;
	struct pcap_pkthdr *header;
	const u_char *packet;
	int i;
	
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
		return 0;
	}
	
	pcap_next_ex(pcap, &header, &packet);
	
	for (i = 0; i < sizeof(packet); i++)
		printf("%X", packet[i]);
	
	/*
	while (pcap_next_ex(pcap, &header, &data) {
	}
	*/
	
	
	
	return 0;
}
