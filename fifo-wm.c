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
int padding;
int layout;
Display	*display;
Window root; 
struct timeval tv;
long unfocusedColor;
long focusedColor; 

/* Tracks the current container */
void crawlContainer(Container * container, int level) {
	Container *c;
	for (c = container; c != NULL; c = c -> next) {
		int j;
		for (j = level; j > 0; j--) { fprintf(stderr, "\t"); }
		char *or = c -> layout == 0 ? "Vertical" : "Horizontal";
		fprintf(stderr, "[%d]=> Container (%s)\n", level, or);
		if (c -> client != NULL) {
			Client *d;
			for (d = c -> client; d != NULL; d = d -> next) {
				int h;
				for (h = level + 1; h > 0; h--) { fprintf(stderr, "\t"); }
				fprintf(stderr, "Client\n");
			}
		}
		if (c -> child != NULL) {
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
		fprintf(stderr, "Adding Token %s\n", token);
	}


	fprintf(stderr, "Last Token %s");

	if (!strcmp(tokens[0], "kill")) {
		fprintf(stderr, "Killing Client");
	} else if (!strcmp(tokens[0], "dump")) {
		dumpTree();
	} else if (!strcmp(tokens[0], "layout")) {
		fprintf(stderr, "Setting layout to: %s", tokens[1]);
		if (!strcmp(tokens[1], "vertical")) {	
			lastContainer -> layout = 0;
		} else if (!strcmp(tokens[1], "horizontal")) {
			lastContainer -> layout = 1;
		}
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
		fprintf(stderr, "Addng client\n");
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
		XSetWindowBorderWidth(display, a -> window, 1);
		XSetWindowBorder(display, a -> window, unfocusedColor);

		switch (container -> layout) {
			case 0:
				XMoveResizeWindow(display, a -> window, 
						(x + (i * (width / children))) + padding, 
						y + padding, 
						(width / children) + padding, 
						height + padding);
				break;
			case 1:
				XMoveResizeWindow(display, a -> window, 
						x + padding, 
						(y + (i * (height / children))) + padding, 
						width + padding, 
						(height / children) + padding);
				break;
			default:
				break;
		}

		return 0;
	}
}


//Thank you DWM ;)
unsigned long getColor(const char *colstr) {
	Colormap cmap = DefaultColormap(display, activeScreen);
	XColor color;

	if(!XAllocNamedColor(display, cmap, colstr, &color, &color)) { return 0; }
	return color.pixel;
}

void focusClient(Client * client) {
	XSetWindowBorderWidth(display, client -> window, 2);
	XSetWindowBorder(display, client -> window, focusedColor);
}


/* ====================
 * Handling of X Events 
 * ==================== */
void xMapRequest(XEvent *event) {
	Client *newClient; /* Create and then parent the cusecusecuseclient */
	newClient             = malloc(sizeof(Client));
	newClient -> window   = event -> xmaprequest.window;
	fprintf(stderr, "Got a map request\n");


	//Create a new container, parent it in last container, parent client in this new container
	Container * newContainer = malloc(sizeof(Container));

	newContainer -> layout = layout;
	parentClient(newClient, newContainer);
	parentContainer(newContainer, lastContainer);

	lastContainer = newContainer;

	XSetWindowBorderWidth(display, newClient -> window, 5);
	XSetWindowBorder(display, newClient -> window, unfocusedColor);

	//Update view
	placeContainer(
			currentContainer, 0, 0, 
			DisplayWidth  (display, activeScreen),
			DisplayHeight (display, activeScreen)
			);

	focusClient(newClient);


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

	//1/5 Second Interval
	tv.tv_sec = 0;  
	tv.tv_usec = 200000;

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
	return 0;
}
