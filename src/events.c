#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "events.h"

void handleXEvent(XEvent *event) {
	switch (event -> type) {
		case MapRequest:     eMapRequest(event);    break;
		case ButtonPress:    eButtonPress(event);   break;
		default:                                    break;
	}
}

void eMapRequest(XEvent *event) {
	Client *newClient; 
	newClient             = malloc(sizeof(Client));
	newClient -> window   = event -> xmaprequest.window;
	fprintf(stderr, "Got a map request\n");

	fprintf(stderr, "\n\nMap Request Window is %d\n\n", event -> xmaprequest.window);

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

	focusWindow(&(newClient -> window));

	//Add Client and window to lookup list
	Lookup *entry = malloc(sizeof(Lookup));
	entry -> client = newClient;
	int win = event -> xmaprequest.window; 
	entry -> window = win;
	entry -> previous = lookup;
	lookup = entry;
}

void eButtonPress(XEvent *event) {

	fprintf(stderr, "\n\nButton Event Window is %p\n\n", &(event -> xbutton.subwindow));

	//Root Window
	if (event -> xbutton.subwindow == None) { return; }

	Client *c = getClientByWindow(&(event -> xbutton.subwindow));

	fprintf(stderr, "Got the client matching to the window %d", c);
	//focusWindow( & (event -> xbutton.subwindow));
}
