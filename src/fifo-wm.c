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
	int fifoFd = 0;
	int xFd = ConnectionNumber(display);
	fd_set descriptors; //Descriptors FD Set

	char commands[256];

	int result = 0;

	struct timeval tv;
	tv.tv_sec = 200;  

	//Must open RDWR so select works properly
	fifoFd = open(FIFO, O_RDWR | O_NONBLOCK);
	for (;;) {
		FD_ZERO(&descriptors); 
		FD_SET(xFd, &descriptors); 
		FD_SET(fifoFd, &descriptors);

		if (select(fifoFd + 1, &descriptors, 0, 0, &tv)) {

			if ((result = read(fifoFd, commands, sizeof(commands))) > 0) {
				commands[result] = '\0';
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
	layout = CONTAINER_DEFAULT_LAYOUT;
	containerPadding = CONTAINER_PADDING;
	clientPadding = CLIENT_PADDING;

	// Open Display and set acitveScreen
	assert((display = XOpenDisplay(NULL)));
	activeScreen = DefaultScreen(display);

	// Setup Root / Screen Padding
	root       = RootWindow(display, activeScreen);
	rootX      = SCREEN_PADDING_LEFT;
	rootY      = SCREEN_PADDING_TOP;
	rootWidth  = DisplayWidth(display, activeScreen) - SCREEN_PADDING_LEFT - SCREEN_PADDING_RIGHT;
	rootHeight = DisplayHeight(display, activeScreen) - SCREEN_PADDING_TOP - SCREEN_PADDING_BOTTOM;
	XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
	setCursor(&root, 68);

	// Setup Clients Defaults
	border         = CLIENT_BORDER_WIDTH;
	focusedColor   = getColor(CLIENT_FOCUSED_COLOR);
	selectedColor  = getColor(CLIENT_SELECTED_COLOR);
	unfocusedColor = getColor(CLIENT_UNFOCUSED_COLOR);

	// Setup the Root Node (top of tree)
	rootNode = allocateNode();
	rootNode -> layout = layout;
	rootNode -> x = rootX; rootNode -> y = rootY;
	rootNode -> width = rootWidth; rootNode-> height = rootHeight;
	viewNode = rootNode;

	// Set Error Handlers and Flush to X
	XSetErrorHandler((XErrorHandler)(xError));
	XFlush(display);
}


int main(void) {
	setup();
	handleEvents();

	return 0;
}
