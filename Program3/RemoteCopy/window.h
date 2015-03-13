#ifndef WINDOW_H
#define WINDOW_H

#include <sys/types.h>
#include "packet.h"

typedef struct window Window;

struct window {
	int32_t bottom;
	int32_t middle;
	int32_t top;
	int32_t size;
	int32_t end;
	
	Packet *buffer;
	int *valid;
};

void init_window(Window *, int);

void add_to_buffer(Window *window, Packet *packet);

void get_from_buffer(Window *window, Packet *packet, int seq_num);

void remove_from_buffer(Window *window, Packet *packet, int seq_num);

void slide(Window *window, int new_bottom);

void destroy_window(Window *window);

int is_closed(Window *);

#endif
