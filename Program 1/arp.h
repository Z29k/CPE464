#ifndef ARP_H
#define ARP_H

struct arp_obj;

int arp_init(struct arp_obj **, u_char *, int);
void arp_print(struct arp_obj *);
void arp_free(struct arp_obj **);

#endif