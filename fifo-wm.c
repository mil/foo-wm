#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <fcntl.h> 
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "config.h"


int screen, activeScreen;
Display	*display;
Window root; 
struct timeval tv;

/* Tracks the current container */
Client *currentClient; //Current client
Client *activeClient; // The active client

/* =========================
 * Handling of FIFO Commands 
 * ========================= */
void handleCommand(char* request) {
	fprintf(stderr, "Recv from FIFO: %s", request);

	//Array to contain pointers to tokens, max of 5 tokens
	char *tokens[5];

	int counter = 0;
	char *lastToken, *token;
	for ( token = strtok_r(request, " ", &lastToken); token; token = strtok_r(NULL, " ", &lastToken)) {
		tokens[counter++] = token;
		fprintf(stderr, "Adding Token %s", token);
	}

	if (!strcmp(tokens[0], "kill")) {
		fprintf(stderr, "Killing Client");
	}
}


/* ===================
 * Tree Related
 * =================== */
int reparent(Client * child, Client * parent) {
	/* First client to be added to container */
	if (parent -> child == NULL) {
		parent -> child = child;

	} else {
		/* Find last child added */
		Client *c = parent -> child;
		while (c -> next != NULL) { 
			c = c -> next; 
		}

		/* Link new child */
		c -> next = child;
		child -> previous = c;
	}

	view(currentClient);
}

/* Views client, provides intial call to recursive fn placeClient */
int view(Client * client) {
	fprintf(stderr, "In view\nCalling placeClient()\n");
	placeClient(
			client, 0, 0, 
			DisplayWidth  (display, activeScreen),
			DisplayHeight (display, activeScreen)
			);
}

int placeClient(Client * client, int x, int y, int width, int height) {
	/* Reached the end, place */
	if (! client -> child) {
		XMapWindow(display, client -> window);
		XMoveResizeWindow(display, client -> window, x, y, width, height);
	} else {

		int children;
		Client *c = malloc(sizeof(Client));


		for (c = client -> child; c != NULL; c = c -> next) {
			children++;
		}

		fprintf(stderr, "I have %d children\n", children);

		int i = 0;
		switch (client -> layout) {
			case 0: //Vertical
				for (c = client -> child; c != NULL; c = c -> next, i++) {
					placeClient(
							c,
							x + (i * (width/children)),
							y,
							(width / children),
							height);
				}
				break;
			case 1: //Horizontal
				break;
			case 2: //Tabbed
				break;
			case 3: //Full Screen
				break;
			default:
				break;
		}
	}

	return 0;
}



/* ====================
 * Handling of X Events 
 * ==================== */
void xMapRequest(XEvent *event) {
	Client *newClient; /* Create and then parent the client */
	newClient             = malloc(sizeof(Client));
	newClient -> window   = event -> xmaprequest.window;
	fprintf(stderr, "Got a map request\n");
	reparent(newClient, currentClient);
}


void handleXEvent(XEvent *event) {
	switch (event -> type) {
		case MapRequest:     xMapRequest(event);    break;
		default:                                    break;
	}
}


void handleEvents() {
	int fifoFd, xFd; //File Descriptors for FIFO and X
	fd_set descriptors; //Descriptors FD Set
	XEvent event; 

	char commands[256];

	int result;
	int count = 0;

	//1 Second Interval
	tv.tv_sec = 1;  

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
	currentClient = malloc(sizeof(Client));
	currentClient -> layout = 0;

	display = XOpenDisplay(NULL);
	assert(display);

	root = RootWindow(display, activeScreen);
	activeScreen = DefaultScreen(display);

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


	XFlush(display);
	handleEvents();
	return 0;
}
