#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "events.h"
#include "tree.h"
#include "window.h"
#include "client.h"


void eMapRequest(XEvent *event) {
	Client *newClient; 
	newClient             = malloc(sizeof(Client));
	newClient -> window   = event -> xmaprequest.window;
	fprintf(stderr, "Got a map request\n");

	parentClient(newClient, currentContainer);

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

void eDestroyNotify(XEvent *event) {
	fprintf(stderr, "DESTROY NOTIFY RECIEVED");

	Client *c = getClientByWindow(&(event -> xdestroywindow.window));
	destroyClient(c);

	//Update view
	placeContainer(
			rootContainer, 0, 0, 
			DisplayWidth  (display, activeScreen),
			DisplayHeight (display, activeScreen)
			);
}

void eConfigureRequest(XEvent *event) {
	fprintf(stderr, "Receiveced a Resize Request EVENT\n");
	Client *c = getClientByWindow(&(event -> xconfigurerequest.window));
	XConfigureRequestEvent *configure = &(event -> xconfigurerequest);

	XWindowChanges changes = { configure -> x, configure -> y, 
		configure -> width, configure -> height };
	XUnmapWindow(display, c -> window);
	XConfigureWindow(display, c -> window, configure -> value_mask, &changes);
	XMoveResizeWindow(display, c-> window, c -> x, c -> y, c -> width, c -> height);
	XMapWindow(display, c -> window);
	
}

void eButtonPress(XEvent *event) {
	fprintf(stderr, "Button Event Window is %p\n", &(event -> xbutton.subwindow));

	//Root Window
	if (event -> xbutton.subwindow == None) { return; }

	Client *c = getClientByWindow(&(event -> xbutton.subwindow));
	focusClient(c);
}

void handleXEvent(XEvent *event) {
	switch (event -> type) {
		case MapRequest:        eMapRequest(event);        break;
		case DestroyNotify:     eDestroyNotify(event);     break;
		case ConfigureRequest:  eConfigureRequest(event);  break;
		case ButtonPress:       eButtonPress(event);       break;
		default:                                           break;
	}
}

