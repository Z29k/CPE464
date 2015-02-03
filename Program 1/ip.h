#ifndef IP_H
#define IP_H

#define ICMP_PROTOCOL 1
#define UDP_PROTOCOL 17
#define TCP_PROTOCOL 6

struct ip_obj;

int ip_init(struct ip_obj **, u_char *, int);
void ip_print(struct ip_obj *);
void ip_print2(struct ip_obj *);
void ip_free(struct ip_obj **);
int ip_protocol(struct ip_obj *);
u_char *ip_data(struct ip_obj *);
int ip_data_length(struct ip_obj *);
u_char *ip_psuedo_header(struct ip_obj *);

#endif