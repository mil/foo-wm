#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <fcntl.h> 
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "config.h"


Container * currentContainer;
Container * lastContainer;

int screen, activeScreen;
int lastOrientation;
Display	*display;
Window root; 
struct timeval tv;

/* Tracks the current container */
void crawlContainer(Container * container, int level) {
	Container *c;
	for (c = container; c != NULL; c = c -> next) {
		int j;
		for (j = level; j > 0; j--) { fprintf(stderr, "\t"); }
		fprintf(stderr, "[%d]=> Container\n", level);
		if (c -> child == NULL) {
			Client *d;
			for (d = c -> client; d != NULL; d = d -> next) {
				int h;
				for (h = level + 1; h > 0; h--) { fprintf(stderr, "\t"); }
				fprintf(stderr, "Client\n");
			}
		} else {
			crawlContainer(c -> child, level + 1);
		}
	}
}


void dumpTree() {
	fprintf(stderr, "Printing the tree\n");
	crawlContainer(currentContainer , 0);
}


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
	} else if (!strcmp(tokens[0], "dump")) {
		dumpTree();
	}

}


/* ===================
 * Tree Related
 * =================== */

int parentClient(Client * child, Container * parent) {
	/* First client to be added to container */
	if (parent == NULL) { return 0; }

	child -> parent = parent;

	if (parent -> client == NULL) {
		fprintf(stderr, "Addng client");
		parent -> client = child;

	} else {
		Client *c = parent -> client;
		while (c -> next != NULL) { c = c -> next; }
		c -> next = child;
		child -> previous = c;
		child -> parent = parent;
	}
}

int parentContainer(Container * child, Container * parent) {
	if (parent == NULL) { return 0; }

	child -> parent = parent;

	if (parent -> child == NULL) {
		parent -> child = child;
	
	} else {
		Container *c = parent -> child;
		while (c -> next != NULL) { c = c -> next; }
		c -> next = child;
		child -> previous = c;
		child -> parent = parent;
	}
}

int placeContainer(Container * container, int x, int y, int width, int height) {

	//Count up children containers
	int containerChildren = 0;
	Container *b = malloc(sizeof(Container));
	for (b = container -> child; b != NULL; b = b -> next) { containerChildren++; }

	//Count up children clients
	int clientChildren = 0;
	Client *a = malloc(sizeof(Client));
	for (a = container -> client; a != NULL; a = a -> next) { clientChildren++; }

	int children = containerChildren + clientChildren;

	int i = 0;
	for (b = container -> child; b != NULL; b = b -> next, i++) {
		switch (container -> layout) {
			case 0:
				placeContainer(b,
						x + (i * (width/children)), y,
						(width / children), height);
				break;

			case 1:
				placeContainer(b,
						x, y + (i * (height/children)),
						width, (height / children));
				break;

			default:
				break;
		}
	}
	for (a = container -> client; a != NULL; a = a -> next, i++) {
		XMapWindow(display, a -> window);
		switch (container -> layout) {
			case 0:
				XMoveResizeWindow(display, a -> window, 
						x + (i * (width / children)), y, 
						(width / children), height);
				break;
			case 1:
				XMoveResizeWindow(display, a -> window, 
						x, y + (i * (height / children)), 
						width, (height / children));
				break;
			default:
				break;
		}

		return 0;
	}
}

/* ====================
 * Handling of X Events 
 * ==================== */
void xMapRequest(XEvent *event) {
	Client *newClient; /* Create and then parent the client */
	newClient             = malloc(sizeof(Client));
	newClient -> window   = event -> xmaprequest.window;
	fprintf(stderr, "Got a map request\n");


	//Create a new container, parent it in last container, parent client in this new container
	Container * newContainer = malloc(sizeof(Container));

	lastOrientation = (lastOrientation == 0) ? 1 : 0;
	newContainer -> layout = lastOrientation;
	parentClient(newClient, newContainer);
	parentContainer(newContainer, lastContainer);
	dumpTree();

	lastContainer = newContainer;

	//Update view
	placeContainer(
			currentContainer, 0, 0, 
			DisplayWidth  (display, activeScreen),
			DisplayHeight (display, activeScreen)
			);

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

int xError(XErrorEvent *e) {
	char err[500];

	XGetErrorText(display, e -> request_code, err, 500);
	fprintf(stderr, "XErrorEvent of Request Code: %d and Error Code of %d\n", e -> request_code, e -> error_code);
	fprintf(stderr, "%s\n", err);
	return 0;
}



int main() {
	lastOrientation = 1;
	currentContainer = malloc(sizeof(Container));
	currentContainer -> layout = 0;

	lastContainer = currentContainer;

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


	XSetErrorHandler((XErrorHandler)(xError));

	XFlush(display);
	handleEvents();
	return 0;
}
