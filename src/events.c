#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "events.h"
#include "tree.h"
#include "window.h"
#include "client.h"

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

	parentClient(newClient, currentContainer);
	currentClient = newClient;

	//Update view
	placeContainer(
			rootContainer, 0, 0, 
			DisplayWidth  (display, activeScreen),
			DisplayHeight (display, activeScreen)
			);

	focusClient(newClient);

	//Add Client and window to lookup list
	Lookup *entry = malloc(sizeof(Lookup));
	entry -> client = newClient;
	int win = event -> xmaprequest.window; 
	entry -> window = win;
	entry -> previous = lookup;
	lookup = entry;
}

void eButtonPress(XEvent *event) {
	fprintf(stderr, "Button Event Window is %p\n", &(event -> xbutton.subwindow));

	//Root Window
	if (event -> xbutton.subwindow == None) { return; }

	Client *c = getClientByWindow(&(event -> xbutton.subwindow));
	focusClient(c);
}
