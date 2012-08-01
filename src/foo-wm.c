#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <fcntl.h> 
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "config.h"
#include "foo-wm.h"
#include "events.h"
#include "tree.h"
#include "commands.h"
#include "util.h"
#include "window.h"

void handleEvents(void) {
	XEvent event; 	
	socklen_t returnAddressSize;
	int socketFd, xFd, socketReturnFd;
	struct sockaddr_un socketAddress, returnAddress;
	char commands[256];  int commandsLength;

	/* Setup the X FD */
	xFd = ConnectionNumber(display);

	/* Get PID, Setup Socket Family, And Setup Address based on PID */
	socketAddress.sun_family = AF_UNIX;
	char * socketName = SOCKET_NAME;
	if (!strcmp(socketName, "NONE")) {
		pid_t pid = getpid();
		socketName = sprintf(socketName, "foo-wm-%d.socket", pid);
	}
	sprintf(socketAddress.sun_path, "%s%s", SOCKET_PATH, socketName);
	unlink(socketAddress.sun_path);

	/* Setup Socket FD, Bind and Listen */
	socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
	bind(socketFd, (struct sockaddr *)&socketAddress, sizeof(socketAddress));
	listen(socketFd, 5);

	/* Assemble the file descriptor set */
	fd_set descriptors; 
	for (;;) {
		FD_ZERO(&descriptors); 
		FD_SET(xFd, &descriptors); 
		FD_SET(socketFd, &descriptors);

		if (select(socketFd + 1, &descriptors, NULL, NULL, NULL)) {

			/* Anything on the Socket File Descriptor? */
			if (FD_ISSET(socketFd, &descriptors)) {
				/* Deal with events from the socket */
				socketReturnFd = accept(
						socketFd, 
						(struct sockaddr*)&returnAddress, 
						&returnAddressSize);
				fprintf(stderr, "\n\nthe socket return FD IS %d\n\n", socketReturnFd);
				if (socketReturnFd != -1) {
					if ((commandsLength = recv(socketReturnFd, commands, sizeof(commands), 0)) > 1) {
						commands[commandsLength] = '\0';
						fprintf(stderr, "Recieved the message %s, from the socket\n", commands);
						handleCommand(commands);

					}
				}
			}

			/* Anything on the X File Descriptor? */
			if (FD_ISSET(xFd, &descriptors)) {
				while (XPending(display)) {
					XNextEvent(display, &event);
					handleXEvent(&event);
				}
			}


		}
	}
	close(socketFd);
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
