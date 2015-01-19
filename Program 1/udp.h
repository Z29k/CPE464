#ifndef UDP_H
#define UDP_H

struct udp_obj;

int udp_init(struct udp_obj **, u_char *, int);
void udp_print(struct udp_obj *);
void udp_free(struct udp_obj **);

#endif