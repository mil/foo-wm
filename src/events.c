#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "foo-wm.h"
#include "commands.h"
#include "events.h"
#include "tree.h"
#include "window.h"
#include "lookup.h"
#include "util.h"


void eMapRequest(XEvent *event) {
	fprintf(stderr, "Got a map request\n");
	Node *newNode = allocateNode();
	newNode -> window = event -> xmaprequest.window;

	/* For Click to Focus */
	XGrabButton(display, AnyButton, AnyModifier, newNode -> window,
			True, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeSync,
			None, None);

	fprintf(stderr, "\n\nIFFFFF\n\n");
	//Containerize and move the viewNode
	if (focusedNode == viewNode) {
		fprintf(stderr, "Focused node is equal to the viewnode\n");
		containerize();
		viewNode = viewNode -> parent;

		//This is the case in which we reparented the root node
		if (focusedNode == rootNode) { rootNode = viewNode; }

		//Brother the new node and rerender
		brotherNode(newNode, viewNode -> child, 1);
		placeNode(viewNode,  rootX, rootY, rootWidth, rootHeight);
	} else if (focusedNode && focusedNode -> parent) {
		brotherNode(newNode, focusedNode, 1);
		placeNode(focusedNode,
				focusedNode -> parent -> x, focusedNode -> parent -> y,
				focusedNode -> parent -> width, focusedNode -> parent -> height);
	} else {
		//No focus node, fist element created
		fprintf(stderr, "FIRST NODE YO\n");

		parentNode(newNode, viewNode);
	}

	fprintf(stderr, "\n\nAFTA\n\n");

	addLookupEntry(newNode, &newNode -> window);
	fprintf(stderr, "added the lookup entry\n");
	focusNode(newNode, NULL, True, True);
	fprintf(stderr, "done with the map request\n");
}

void eDestroyNotify(XEvent *event) {
	fprintf(stderr, "DESTROY NOTIFY RECIEVED");

	Node *n = getNodeByWindow(&(event -> xdestroywindow.window));
	if (n == NULL) return;

	if (n == viewNode) { 
		viewNode = n -> parent; 
		fprintf(stderr, "Equals view node\n");
	}
	destroyNode(n);
	placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
}

void eConfigureRequest(XEvent *e) {

	/* Structed From DWM */
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	Node *configuredNode = getNodeByWindow(&ev->window);

	if (!configuredNode) return;

	XWindowChanges wc;
	wc.x = configuredNode -> x; 
	wc.y = configuredNode -> y;
	wc.width = ev->width;       
	wc.height = ev->height;
	wc.border_width = ev->border_width;
	wc.sibling = ev->above;
	wc.stack_mode = ev->detail;
	XConfigureWindow(display, ev->window, ev->value_mask, &wc);

	placeNode(configuredNode, 
			configuredNode -> x, configuredNode -> y,
			configuredNode -> width, configuredNode -> height);

}

void eResizeRequest(XEvent *event) {

}

void eButtonPress(XEvent *event) {
	fprintf(stderr, "Button Event Window is %p\n", &(event -> xbutton.window));

	// Root Window
	if (event -> xbutton.window == None) return;

	// Click to Focus
	focusNode(getNodeByWindow(&(event -> xbutton.window)), event, True, True);
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
