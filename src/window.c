#include <X11/Xlib.h>

#include "foo-wm.h"
#include "window.h"

void centerPointer(Window *window) {
	//Get Window Attributes
	XWindowAttributes windowAttributes;
	XGetWindowAttributes(display, *window, &windowAttributes);

	int centerX = windowAttributes.width  / 2,
			centerY = windowAttributes.height / 2;

	//Warp to Center
	XWarpPointer(display, None, *window, 0, 0, 0, 0, centerX,centerY);
}

void setCursor(Window *window, int cursor) {
	cursor = XCreateFontCursor(display, cursor);
	XDefineCursor(display, *window, cursor);
}
