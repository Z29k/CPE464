#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "packet.h"
#include "window.h"

void init_window(Window *window, int size) {
	window->bottom = 1;
	window->middle = 1;
	window->top = size;
	window->size = size;
	
	window->buffer = calloc(size, sizeof(Packet));
	window->valid = calloc(size, sizeof(int));
}

int is_closed(Window *window) {
	return window->middle > window->top;
}

void add_to_buffer(Window *window, Packet *packet) {
	int index;
	
	index = (packet->seq_num - 1) % window->size;
	
	memcpy(window->buffer + index, packet, sizeof(Packet));
	window->valid[index] = 1;
}

void get_from_buffer(Window *window, Packet *packet, int seq_num) {
	int index;
	
	index = (seq_num - 1) % window->size;
	
	memcpy(packet, window->buffer + index, sizeof(Packet));
	
	if (window->valid[index] == 0)
		printf("!!!!!!!!!get_from_buffer invalid packet!!!!!!!!!!\n");
}

void remove_from_buffer(Window *window, Packet *packet, int seq_num) {
	int index;
	
	index = (seq_num - 1) % window->size;
	
	if (window->valid[index] == 0) {
		printf("!!!!!!!!!remove_from_buffer invalid packet %d\n", packet->seq_num);
	}
	
	memcpy(packet, window->buffer + index, sizeof(Packet));
	window->valid[index] = 0;
	
}

void slide(Window *window, int new_bottom) {
	int i;
	
	for (i = window->bottom; i < new_bottom; i++)
		window->valid[(i - 1) % window->size] = 0;
	
	window->bottom = new_bottom;
	window->top = new_bottom + window->size - 1;
	
	window->middle = window->middle < window->bottom ? window->bottom : window->middle;
	
	printf("Window slid to %d %d %d\n", window->bottom, window->middle, window->top);
}

int is_valid(Window *window, int seq_num) {
	return window->valid[(seq_num - 1) % window->size];
}

int all_valid(Window *window) {
	int i;
	
	for (i = window->bottom; i <= window->top; i++)
		if (window->valid[(i - 1) % window->size] == 0)
			return 0;
	
	return 1;
}

int is_in_window(Window *window, int seq_num) {
	return seq_num >= window->bottom && seq_num <= window->top;
}

void destroy_window(Window *window) {
	free(window->buffer);
	free(window->valid);
}
