#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "events.h"
#include "tree.h"
#include "window.h"


void eMapRequest(XEvent *event) {
	fprintf(stderr, "Got a map request\n");
	Node *newNode;
	newNode = malloc(sizeof(Node));
	newNode -> window = event -> xmaprequest.window;

	if (activeNode == NULL) {
		parentNode(newNode, viewNode);
	} else {
		parentNode(newNode, activeNode -> parent);
	}


	fprintf(stderr, "The focused node (BEFORE) is %p\n", activeNode);
	focusNode(newNode);
	fprintf(stderr, "The focused node (AFTER) is %p\n", activeNode);



	//Update the view
	placeNode( activeNode -> parent, 
			(activeNode -> parent) -> x, 
			(activeNode -> parent) -> y, 
			(activeNode -> parent) -> width, 
			(activeNode -> parent) -> height);



	//Add Client and window to lookup list
	Lookup *entry = malloc(sizeof(Lookup));
	entry -> node= newNode;
	int win = event -> xmaprequest.window; 
	entry -> window = win;
	entry -> previous = lookup;
	lookup = entry;
}

void eDestroyNotify(XEvent *event) {
	fprintf(stderr, "DESTROY NOTIFY RECIEVED");

	Node *n = getNodeByWindow(&(event -> xdestroywindow.window));
	if (n == NULL) { return; }

	fprintf(stderr," Getting cloest node to %p\n", n);
	activeNode = getClosestNode(n);
	fprintf(stderr, "Got the cloest node, is: %p\n", activeNode);

	destroyNode(n);
	fprintf(stderr, "Passed the destroyu node\n");
	fprintf(stderr, "Yo my viewnode is %p", viewNode);

	//Update view
	placeNode( viewNode, rootX, rootY, rootWidth, rootHeight);
}

void eConfigureRequest(XEvent *event) {
	fprintf(stderr, "Receiveced a Resize Request EVENT\n");

	Node *n = getNodeByWindow(&(event -> xconfigurerequest.window));
	if (n != NULL) {
		XUnmapWindow(display, n -> window);
		XMoveResizeWindow(display, n -> window, n -> x, n -> y, n -> width, n -> height);
		XMapWindow(display, n -> window);
	}
	
}

void eResizeRequest(XEvent *event) {

}

void eButtonPress(XEvent *event) {
	fprintf(stderr, "Button Event Window is %p\n", &(event -> xbutton.subwindow));

	//Root Window
	if (event -> xbutton.subwindow == None) { return; }

	Node *n = getNodeByWindow(&(event -> xbutton.subwindow));
	focusNode(n);
}

void handleXEvent(XEvent *event) {
	switch (event -> type) {
		case MapRequest:        eMapRequest(event);        break;
		case DestroyNotify:     eDestroyNotify(event);     break;
		case ConfigureRequest:  eConfigureRequest(event);  break;
		case ResizeRequest:     eResizeRequest(event);     break;
		case ButtonPress:       eButtonPress(event);       break;
		default:                                           break;
	}
}
