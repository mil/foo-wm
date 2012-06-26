#include <stdio.h>
#include <stdlib.h>
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
	int fifoFd, xFd; //File Descriptors for FIFO and X
	fd_set descriptors; //Descriptors FD Set
	XEvent event; 

	char commands[256];

	int result;
	int count = 0;

	//1/5 Second Interval
	tv.tv_sec = 0;  
	tv.tv_usec = 50000;

	for (;;) {
		/* Reset the File Descriptor */
		FD_ZERO(&descriptors); 

		/* Open the FIFO FD, Add the X FD and the FIFO FD to the Set */
		fifoFd = open(FIFO, O_RDWR | O_NONBLOCK);
		FD_SET(fifoFd, &descriptors);
		FD_SET(xFd, &descriptors); 

		/* Run Select on File Descriptors */
		select(xFd + 1, &descriptors, 0, 0, &tv);

		//Recieved event from X
		while (XPending(display)) {
			XNextEvent(display, &event);
			handleXEvent(&event);
		}

		//Commands from FIFO can be up to 300 character long
		if ((result = read(fifoFd, commands, 300)) > 0) {
			commands[result] = '\0';
			handleCommand(commands);
		}
		close(fifoFd);
	}
}
  

int main() {
	layout = CONTAINER_DEFAULT_LAYOUT;
	padding = CONTAINER_PADDING;

	currentContainer = malloc(sizeof(Container));
	currentContainer -> layout = layout;

	lastContainer = currentContainer;

	display = XOpenDisplay(NULL);
	assert(display);

	root = RootWindow(display, activeScreen);
	activeScreen = DefaultScreen(display);


	focusedColor = getColor(CLIENT_FOCUSED_COLOR);
	unfocusedColor = getColor(CLIENT_UNFOCUSED_COLOR);



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


	XSetErrorHandler((XErrorHandler)(xError));

	XFlush(display);
	handleEvents();

	free(currentContainer);
	return 0;
}
