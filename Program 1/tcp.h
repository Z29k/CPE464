#ifndef TCP_H
#define TCP_H

struct tcp_obj;

int tcp_init(struct tcp_obj **, u_char *, u_char *, int);
void tcp_print(struct tcp_obj *);
void tcp_free(struct tcp_obj **);

#endif