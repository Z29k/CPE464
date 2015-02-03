#ifndef ETHERNET_H
#define ETHERNET_H

#define IP_TYPE_NO 0x0800
#define ARP_TYPE_NO 0x0806
#define UNKNOWN_TYPE_NO 0

struct ethr_obj;

int ethr_init(struct ethr_obj **, struct pcap_pkthdr *, const u_char *);
void ethr_print(struct ethr_obj *);
void ethr_print2(struct ethr_obj *);
void ethr_free(struct ethr_obj **);
u_char *ethr_data(struct ethr_obj *);
int ethr_data_length(struct ethr_obj *);
int ethr_type(struct ethr_obj *);

#endif