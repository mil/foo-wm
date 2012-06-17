#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <fcntl.h> 
#include <string.h>
#include <assert.h>
#define FIFO "wm-fifo"

int screen, activeScreen;
Display	*display;
Window root; 
struct timeval tv;

typedef struct Client Client;
struct Client {
	Window window;
	Client *previous;
};

typedef struct {
	Window active;
	Client *last;
} Workspace;

Workspace workspaces[10];
int currentWorkspace = 0;

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
	} else if (!strcmp(tokens[0], "workspace")) {
		int workspace = atoi(tokens[1]);
		fprintf(stderr, "Switchin to workspace %d", workspace);
	}
}

/* ====================
 * Handling of X Events 
 * ==================== */
void xMapRequest(XEvent *event) {
	Client *newClient;
	newClient             = malloc(sizeof(Client));
	newClient -> window   = event -> xmaprequest.window;
	newClient -> previous = workspaces[currentWorkspace].last;

	XMapWindow(display, newClient -> window);
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
		fifoFd = open(FIFO, O_RDONLY | O_NONBLOCK);
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
