#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "fifo-wm.h"
#include "tree.h"

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

	return 1;
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

	return 1;
}

int placeContainer(Container * container, int x, int y, int width, int height) {

	//Count up children
	int children = 0;
	Client *a = malloc(sizeof(Client));
	Container *b = malloc(sizeof(Container));
	for (a = container -> client; a != NULL; a = a -> next) { children++; }
	for (b = container -> child;  b != NULL; b = b -> next) { children++; }


	/* Recursive call to placeContainer */
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

	}

	free(a), free(b);
	return 0;
}

//Returns the client associated with given window
Client * getClientByWindow(Window * window) {
	Lookup *node;
	int win = *window;
	for (node = lookup; node != NULL; node = node ->  previous)
		if (win == node -> window)
			return node -> client;

	free(node);
	return NULL;
}
