#include <X11/Xlib.h>
#include <stdio.h>

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
	char err[500];

	XGetErrorText(display, e -> request_code, err, 500);
	fprintf(stderr, "XErrorEvent of Request Code: %d and Error Code of %d\n", e -> request_code, e -> error_code);
	fprintf(stderr, "%s\n", err);
	return 0;
}
