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

void handleEvents(void) {
	XEvent event; 
	int fifoFd = 0, xFd = ConnectionNumber(display), readResult = 0;
	char commands[256]; fd_set descriptors; 
	struct timeval tv; tv.tv_sec = 200;  

	//Must open RDWR so select works properly
	fifoFd = open(FIFO, O_RDWR | O_NONBLOCK);
	for (;;) {
		FD_ZERO(&descriptors); 
		FD_SET(xFd, &descriptors); 
		FD_SET(fifoFd, &descriptors);

		if (select(fifoFd + 1, &descriptors, 0, 0, &tv)) {

			if ((readResult = read(fifoFd, commands, sizeof(commands))) > 0) {
				commands[readResult] = '\0';
				handleCommand(commands);
			}

			while (XPending(display)) {
				XNextEvent(display, &event);
				handleXEvent(&event);
			}
		}
	}
	close(fifoFd);
}

void setup(void) {
	// Setting from defines from config.h
	defaultLayout    = CONTAINER_DEFAULT_LAYOUT;
	containerPadding = CONTAINER_PADDING;
	clientPadding    = CLIENT_PADDING;

	// Open display, set screen, set root, and select root input
	assert((display = XOpenDisplay(NULL)));
	activeScreen    = DefaultScreen(display);
	root            = RootWindow(display, activeScreen);
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	setCursor(&root, 68);

	// Setup Clients Defaults
	border                 = CLIENT_BORDER_WIDTH;
	activeFocusedColor     = getColor(CLIENT_ACTIVE_FOCUSED_COLOR);
	activeUnfocusedColor   = getColor(CLIENT_ACTIVE_UNFOCUSED_COLOR);
	inactiveFocusedColor   = getColor(CLIENT_INACTIVE_FOCUSED_COLOR);
	inactiveUnfocusedColor = getColor(CLIENT_INACTIVE_UNFOCUSED_COLOR);

	// Setup the Root Node (top of tree)
	rootNode                       = allocateNode();
	rootNode -> layout             = defaultLayout;
	rootNode -> x      = rootX     = SCREEN_PADDING_LEFT;
	rootNode -> y      = rootY     = SCREEN_PADDING_TOP;
	rootWidth = DisplayWidth(display, activeScreen) 
		- SCREEN_PADDING_LEFT - SCREEN_PADDING_RIGHT;
	rootNode -> width  = rootWidth;
	rootHeight = DisplayHeight(display, activeScreen) 
		- SCREEN_PADDING_TOP - SCREEN_PADDING_BOTTOM;
	rootNode -> height = rootHeight;
	viewNode           = rootNode;

	// Set Error Handlers and Flush to X
	XSetErrorHandler((XErrorHandler)(xError));
	XFlush(display);
}


int main(void) {
	setup();
	handleEvents();

	return 0;
}
