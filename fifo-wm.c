#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <fcntl.h> 
#include <assert.h>
#define FIFO "wm-fifo"
#define NIL (0)

int screen, activeScreen;
Display	*display;
Window root; 
struct timeval tv;

//For Processing Tokens
void tokenizeCommands(char* request) {
	fprintf(stderr, "Recv from FIFO: %s", request);
	char *lastToken, *token;
	int tokenResponse;
	for ( token = strtok_r(request, " ", &lastToken); 
			token; 
			token = strtok_r(NULL, " ", &lastToken)) {
		fprintf(stderr, "Processing Token %s\n", token);
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
	tv.tv_usec = 0;


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
			fprintf(stderr, "Recieved an X Event\n");
		}

		//Commands from FIFO can be up to 300 character long
		if ((result = read(fifoFd, commands, 300)) > 0) {
			commands[result] = '\0';
			tokenizeCommands(commands);
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

void handleToken(char *token) {
	printf("Handling token: %s\n", token);
	if      (token == "kill") {}
	else if (token == "focus") {}
	else if (token == "workspace") {}
	else if (token == "focus") {}
}
