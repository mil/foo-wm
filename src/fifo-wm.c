#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <fcntl.h> 
#include <string.h>
#include <assert.h>

#include "config.h"
#include "fifo-wm.h"
#include "events.h"
#include "tree.h"
#include "commands.h"
#include "util.h"
#include "window.h"

void handleEvents() {
	XEvent event; 
	int fifoFd = 0;
	int xFd = ConnectionNumber(display);
	fd_set descriptors; //Descriptors FD Set

	char commands[256];

	int result = 0;

	tv.tv_sec = 200;  

	fifoFd = open(FIFO, O_RDONLY | O_NONBLOCK);
	for (;;) {
		FD_ZERO(&descriptors); 
		FD_SET(xFd, &descriptors); 
		FD_SET(fifoFd, &descriptors);

		while (select(fifoFd + 1, &descriptors, 0, 0, &tv)) {

			if ((result = read(fifoFd, commands, 200)) > 0) {
				commands[result] = '\0';
				handleCommand(commands);
			}

			while (XPending(display) > 0) {
				XNextEvent(display, &event);
				handleXEvent(&event);
			}
		}
	}
	close(fifoFd);
}


int main() {
	layout = CONTAINER_DEFAULT_LAYOUT;
	padding = CONTAINER_PADDING;

	display = XOpenDisplay(NULL);
	assert(display);

	root = RootWindow(display, activeScreen);
	activeScreen = DefaultScreen(display);

	rootX = 0 + SCREEN_PADDING_LEFT;
	rootWidth = DisplayWidth(display, activeScreen) - SCREEN_PADDING_LEFT - SCREEN_PADDING_RIGHT;

	rootY = 0 + SCREEN_PADDING_TOP;
	rootHeight = DisplayHeight(display, activeScreen) - SCREEN_PADDING_TOP - SCREEN_PADDING_BOTTOM;


	focusedColor = getColor(CLIENT_FOCUSED_COLOR);
	unfocusedColor = getColor(CLIENT_UNFOCUSED_COLOR);
	border = BORDER_WIDTH;



	XGrabButton(
			display, AnyButton, AnyModifier, 
			root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask | OwnerGrabButtonMask, 
			GrabModeAsync, GrabModeAsync, None, None
			);
	XSelectInput(display, root, 
			FocusChangeMask | PropertyChangeMask |
			SubstructureNotifyMask | SubstructureRedirectMask | 
			KeyPressMask | ButtonPressMask
			);

	viewNode = allocateNode();
	viewNode -> layout = layout;
	viewNode -> x = rootX; viewNode -> y = rootY;
	viewNode -> width = rootWidth; viewNode -> height = rootHeight;

	XSetErrorHandler((XErrorHandler)(xError));
	XFlush(display);
	handleEvents();

	return 0;
}
