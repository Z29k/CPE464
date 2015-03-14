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

int is_valid(Window *window, int seq_num);

int is_in_window(Window *window, int seq_num);

void destroy_window(Window *window);

int empty_buffer(Window *window);

int is_closed(Window *);

void print_window(Window *window);

#endif
