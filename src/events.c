#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "events.h"
#include "tree.h"
#include "window.h"
#include "util.h"


void eMapRequest(XEvent *event) {
	fprintf(stderr, "Got a map request\n");
	Node *newNode = allocateNode();
	newNode -> window = event -> xmaprequest.window;

	if (selectedNode != NULL) {
		fprintf(stderr,"Mapping based on selectedNode\n");
		parentNode(newNode, selectedNode);

		placeNode(selectedNode,
				selectedNode -> x, selectedNode -> y, 
				selectedNode -> width, selectedNode -> height);


	} else if (focusedNode != NULL) {
		fprintf(stderr,"Mapping based on focusedNode\n");

		//Brother new node to current focus then focus new node
		brotherNode(newNode, focusedNode, 1);
		focusNode(newNode);

		//Rerender parent of (old focus & new node)
		placeNode( focusedNode -> parent, 
				(focusedNode -> parent) -> x, 
				(focusedNode -> parent) -> y, 
				(focusedNode -> parent) -> width, 
				(focusedNode -> parent) -> height);

	} else {
		//All we have to map on is the view node
		fprintf(stderr,"Mapping based on viewNode\n");

		parentNode(newNode, viewNode);
		focusNode(newNode);
		placeNode( viewNode,
				viewNode -> x, viewNode -> y,
				viewNode -> width, viewNode -> height);

	}



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

	focusedNode = getClosestClient(n);
	destroyNode(n);

	//Update view
	placeNode( viewNode, rootX, rootY, rootWidth, rootHeight);
}

void eConfigureRequest(XEvent *event) {
	fprintf(stderr, "Receiveced a Resize Request EVENT\n");

	Node *n = getNodeByWindow(&(event -> xconfigurerequest.window));
	if (n != NULL) {
		XUnmapWindow(display, n -> window);
		XMoveResizeWindow(display, n -> window, 
				n -> x, n -> y, 
				n -> width, n -> height);
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
