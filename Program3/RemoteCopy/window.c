#include <stdlib.h>
#include <string.h>

#include "packet.h"
#include "window.h"

void init_window(Window *window, int size) {
	window->bottom = 0;
	window->middle = 0;
	window->top = size - 1;
	window->size = size;
	
	window->buffer = calloc(size, sizeof(Packet));
	window->valid = calloc(size, sizeof(int));
}

int is_closed(Window *window) {
	return window->middle == window->top;
}

void add_to_buffer(Window *window, Packet *packet) {
	int index;
	
	index = packet->seq_num % window->size;
	
	memcpy(window->buffer + index, packet, sizeof(Packet));
	window->valid[index] = 1;
}

void remove_from_buffer(Window *window, Packet *packet) {
	int index;
	
	index = packet->seq_num % window->size;
	
	memcpy(packet, window->buffer + index, sizeof(Packet));
	window->valid[index] = 0;
}

void destroy_window(Window *window) {
	free(window->buffer);
	free(window->valid);
}
