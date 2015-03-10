#ifndef WINDOW_H
#define WINDOW_H

#include <sys/types.h>

typedef struct window Window;

struct window {
	int32_t bottom;
	int32_t middle;
	int32_t top;
	int32_t size;
	int32_t end;
};

void initWindow(Window *, int);

int isClosed(Window *);

#endif
