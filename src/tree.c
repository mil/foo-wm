#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "fifo-wm.h"
#include "tree.h"

void crawlNode(Node * node, int level) {
	Node *n;
	for (n = node; n != NULL; n = n -> next) {
		int j;
		for (j = level; j > 0; j--) { fprintf(stderr, "|\t"); }

		if (isClient(n -> child)) {
			fprintf(stderr, "Client\n");
		} else {
			char *or = n -> layout == 0 ? "Vertical" : "Horizontal";
			fprintf(stderr, "Container %s\n", or);
			crawlNode(n -> child, level + 1);
		}
	}
}

void dumpTree() {
	int layout;
	fprintf(stderr, "Printing the tree\n");
	crawlNode(viewNode, 0);
}

//Will only work on nodes with windows for now
void focusNode(Node * n) {
	if ((n -> parent) -> focus == n) { return; }

	(n -> parent) -> focus = n;

	XRaiseWindow(display, n -> window);
	XSetInputFocus(display, n -> window, RevertToPointerRoot, CurrentTime);

	placeNode(n -> parent,
			(n -> parent) -> x, (n -> parent) -> y,
			(n -> parent) -> width, (n -> parent) -> height);

}

void destroyNode(Node * n) {
	if (n == NULL) { return; }

	
	if (n -> parent -> child == n && n -> previous == NULL) {
		destroyNode(n -> parent);
		return;
	}


	//Unparent the node
	unparentNode(n);

	//Recursivly destroy all children of the node
	Node * child;
	for (child = n -> child; child != NULL; child -> next) {
		destroyNode(child);
	}

	if (n -> window != (Window)NULL) {
		XUnmapWindow(display, n -> window);
	}
	free(n);
}


void unparentNode(Node *node) {
	if (node -> parent == NULL) { return; }


	if ((node -> parent) -> focus == node) {
		if (node-> next != NULL) { (node -> parent) -> focus = node -> next; }
		if (node-> previous != NULL) { (node -> parent) -> focus = node -> previous; }
	}

	if (node -> next != NULL) {
		(node -> next) -> previous = node -> previous;
		if (node == activeNode) { }
	}

	if (node -> previous != NULL) {
		(node -> previous) -> next = node -> next;
	}

}

void parentNode(Node *node, Node *parent) {
	/* First client to be added to container */
	if (parent == NULL) { return; }

	//Unparent then set the parent to new parent
	unparentNode(node);

	node -> parent = parent;
	parent -> focus = node;

	//Find last in children of parent, add to end
	if (parent -> child == NULL) {
		parent -> child = node;
	} else {
		Node *n = parent -> child;
		while (n -> next != NULL) { n = n -> next; }
		node -> previous = n;
		n -> next = node;
	}
}

void unmapNode(Node * node) {
	Node *n;
	for (n = node -> child; n != NULL; n = n -> next) {
		if (n -> window != (Window) NULL) {
			XUnmapWindow(display, n -> window);
		} else {
			if (n -> child != NULL) {
				unmapNode(n -> child);
			}
		}
	}
}

void placeNode(Node * node, int x, int y, int width, int height) {
	node -> x = x; node -> y = y;
	node -> width = width; node -> height = height;
	fprintf(stderr, "Place Node XY:[%d, %d], WH:[%d, %d]\n", x, y, width, height);

	//if (node -> window != (Window)NULL) {
	if (node -> focus == NULL) {
		XMapWindow(display, node -> window);
		XMoveResizeWindow(display, node -> window, x, y, width, height);
		XSetWindowBorderWidth(display, node -> window, 1);
		if ((node -> parent) -> focus == node) {
			XSetWindowBorder(display, node -> window, focusedColor);
		} else {
			XSetWindowBorder(display, node -> window, unfocusedColor);
		}


	} else {
		//Count up children prior to loop
		int children = 0; int i = 0; Node *a;
		for (a = node -> child; a != NULL; a = a -> next) { children++; }
		for (a = node -> child; a != NULL; a = a -> next, i++) {
			switch (node -> layout) {
				case 0:
					a -> x = x + (i * (width / children)); a -> y = y + padding;
					a -> width = width / children; a -> height = height;
					break;

				case 1:
					a -> x = x; a -> y = y + (i * (height / children));
					a -> width = width; a -> height = height / children;
					break;
			}
			placeNode(a, a -> x, a -> y, a -> width, a -> height);
		}
	}
}

Bool isClient(Node * node) {
	if (node -> window != (Window) NULL)
		return True;
	return False;
}

//Returns the client associated with given window
Node * getNodeByWindow(Window * window) {
	Lookup *entry;
	int win = *window;
	for (entry= lookup; entry != NULL; entry = entry -> previous) {
		if (win == entry -> window)
			return entry -> node;
	}

	return NULL;
}
