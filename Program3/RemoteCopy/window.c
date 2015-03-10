#include "window.h"

void initWindow(Window *window, int size) {
	window->bottom = 0;
	window->middle = 0;
	window->top = size - 1;
	window->size = size;
}

int isClosed(Window *window) {
	return window->middle == window->top;
}
