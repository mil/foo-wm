#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "events.h"
#include "tree.h"
#include "commands.h"
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

	if (selectedNode) {
		fprintf(stderr,"Mapping based on selectedNode\n");

		if (selectedNode -> parent) {
			brotherNode(newNode, selectedNode, 1);
			placeNode(selectedNode -> parent,
					selectedNode -> parent -> x, selectedNode -> parent -> y, 
					selectedNode -> parent -> width, selectedNode -> parent -> height);
		} else {
			fprintf(stderr, "Going to reparent the root node\n");

			containerize();
			rootNode = viewNode = selectedNode -> parent;
			brotherNode(newNode, viewNode -> child, 1);

			placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
		}

	} else if (focusedNode) {
		fprintf(stderr,"Mapping based on focusedNode\n");
		if (focusedNode == viewNode && isClient(viewNode)) {
			fprintf(stderr, "Just viewing a single client");
			containerize();
			viewNode = focusedNode -> parent;
			parentNode(newNode, viewNode);
			placeNode(viewNode,
					rootX, rootY,
					rootWidth, rootHeight);
		} else {
			//Brother new node to current focus then focus new node
			brotherNode(newNode, focusedNode, 1);
			//Rerender parent of (old focus & new node)
			placeNode( focusedNode -> parent, 
					focusedNode -> parent -> x, 
					focusedNode -> parent -> y, 
					focusedNode -> parent -> width, 
					focusedNode -> parent -> height);
		}

	} else if (viewNode) {
		//All we have to map on is the view node
		fprintf(stderr,"Mapping based on viewNode\n");


		parentNode(newNode, viewNode);
		placeNode( viewNode,
				viewNode -> x, viewNode -> y,
				viewNode -> width, viewNode -> height);

	} else {
		fprintf(stderr, "This shouldn't be possible...\n");
	}

	addLookupEntry(newNode, &newNode -> window);
	focusNode(newNode, NULL);
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
	focusNode(getNodeByWindow(&(event -> xbutton.window)), event);
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
