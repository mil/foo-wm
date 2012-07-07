#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "fifo-wm.h"
#include "util.h"

//Thank you DWM ;)
unsigned long getColor(const char *colstr) {
	Colormap cmap = DefaultColormap(display, activeScreen);
	XColor color;

	if(!XAllocNamedColor(display, cmap, colstr, &color, &color)) { return 0; }
	return color.pixel;
}


int xError(XErrorEvent *e) {
	fprintf(stderr, "XErrorEvent of Request Code: %d and Error Code of %d\n", e -> request_code, e -> error_code);
	return 0;
}

Node * allocateNode() {
	Node *n = malloc(sizeof(Node));
	n -> previous = NULL; n -> next = NULL;
	n -> parent = NULL;   n -> child = NULL;
	n -> layout = layout;
	return n;
}

void gridDimensions(int children, int * rows, int * cols) {
	int square = (int) sqrt(children);
	int r = square;

	while (((children % r) != 0)) { r++; }
	int c = children / r;

	if ((r == 1 && c != 1) || (c == 1 && r != 1)) {
		gridDimensions(children + 1, rows, cols);
	} else {
		*rows = r;
		*cols = c;
	}
}


//Returns the client associated with given windowv
Node * getNodeByWindow(Window * window) {
	Lookup *entry;
	int win = *window;
	for (entry = lookup; entry != NULL; entry = entry -> previous) {
		if (win == entry -> window)
			return entry -> node;
	}

	return NULL;
}
