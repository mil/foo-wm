#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "fifo-wm.h"
#include "tree.h"

void crawlNode(Node * node, int level) {
	int j; for (j = level; j > 0; j--) { fprintf(stderr, "|\t"); }

	if (isClient(node)) {
		fprintf(stderr, "Client (%p)", node);
		if (node == activeNode) { fprintf(stderr, " [FOCUS]"); }
		fprintf(stderr, "\n");

	} else {
		char *or = node -> layout == 0 ? "Vertical" : "Horizontal";
		fprintf(stderr, "Container (%p) %s\n", node, or);

		Node *n;
		for (n = node -> child; n != NULL; n = n -> next) {
			crawlNode(n, level + 1);	
		}
	}
}

void dumpTree() {
	fprintf(stderr, "Printing the tree\n");
	fprintf(stderr, "----------------------------------\n");
	crawlNode(viewNode, 0);
	fprintf(stderr, "----------------------------------\n");
}

//Will only work on nodes with windows for now
void focusNode(Node * n) {
	if (activeNode == n) { return; }
	if (!isClient(n)) { 
		fprintf(stderr, "Trying to focus a node thats not a client !\n"); 
		return;
	}
	if (isClient(activeNode)) {
		XSetWindowBorder(display, activeNode -> window, unfocusedColor);
	}

	activeNode = n;
	XRaiseWindow(display, n -> window);
	XSetInputFocus(display, n -> window, RevertToPointerRoot, CurrentTime);
	XSetWindowBorder(display, n -> window, focusedColor);
}

void destroyNode(Node * n) {

	fprintf(stderr, "PRE DESTROY\n");
	dumpTree();
	if (n == NULL) { return; }

	if ( (n -> next == NULL &&
				(n -> parent -> child == n && n -> previous == NULL)) &&
			n -> parent -> parent != NULL
		 ) {
		fprintf(stderr, "Calling destroy node on parent\n");
		destroyNode(n -> parent);
		return;
	}

	//Unparent the node
	unparentNode(n);

	fprintf(stderr, "Sucessfull destroy node\n");
	dumpTree();
	//Recursivly destroy all children of the node
	if (n -> window != (Window)NULL) {
		XUnmapWindow(display, n -> window);
	} else {
		fprintf(stderr, "The child is %p\n", n -> child);
		if (n -> child  -> next != NULL) {
			Node * child;
			for (child = n -> child; child != NULL; child = child -> next) {
				destroyNode(child);
			}
		}

	}
	fprintf(stderr, "POST DESTROY\n");
	dumpTree();

	free(n);
}


void unparentNode(Node *node) {
	if (node == NULL || node -> parent == NULL) { return; }
	fprintf(stderr, "unparent called");

	focusNode(getClosestNode(node));

	//Move parent's child pointer if were it....
	if ((node -> parent) -> child == node) {
		(node -> parent) -> child = node -> next;
	}

	//Move the next and previous pointers to cut out the node
	if (node -> next != NULL) { (node -> next) -> previous = node -> previous; }
	if (node -> previous != NULL) { (node -> previous) -> next = node -> next; }

	//Set our parent to NULL
	node -> parent = NULL; node -> next = NULL; node -> previous = NULL;
}

void parentNode(Node *node, Node *parent) {
	if (parent == NULL) { return; } //Cant add to NULL

	unparentNode(node); //Unparent then set the parent to new parent
	node -> parent = parent;

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
		} else if (n -> child != NULL) {
			unmapNode(n -> child);
		}
	}
}

void placeNode(Node * node, int x, int y, int width, int height) {
	node -> x = x; node -> y = y;
	node -> width = width; node -> height = height;
	fprintf(stderr, "Place Node XY:[%d, %d], WH:[%d, %d]\n", x, y, width, height);

	if (isClient(node)) {
		XMapWindow(display, node -> window);
		XRaiseWindow(display, node -> window);

		XMoveResizeWindow(display, node -> window, 
				x, 
				y, 
				width - (border*2), 
				height - (border *2));
		XSetWindowBorderWidth(display, node -> window, border);
		if (activeNode == node) {
			XSetWindowBorder(display, node -> window, focusedColor);
			focusNode(node);
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
					a -> x = x + (i * (width / children)) + padding; 
					a -> y = y + padding;
					a -> width = (width / children) - (padding*2); 
					a -> height = height - (padding*2);
					break;

				case 1:
					a -> x = x + padding; 
					a -> y = y + (i * (height / children)) + padding;
					a -> width = width - (padding * 2); 
					a -> height = (height / children) - (padding * 2);
					break;
			}
			placeNode(a, a -> x, a -> y, a -> width, a -> height);
		}
	}
}


Bool isClient(Node * node) {
	if (node == NULL) { return False; }
	if (node -> window != (Window) NULL) { return True;
	} else { return False; }
}

//Returns the client associated with given window

Node * getNodeByWindow(Window * window) {
	Lookup *entry;
	int win = *window;
	for (entry = lookup; entry != NULL; entry = entry -> previous) {
		if (win == entry -> window)
			return entry -> node;
	}

	return NULL;
}

Node * getClosestNode(Node * node) {
	Node * pNode = node;
	Node * nNode = node;
	while (pNode -> previous != NULL || nNode -> next != NULL) {
		if (pNode -> previous != NULL) { pNode = pNode -> previous; }
		if (nNode -> next != NULL    ) { nNode = nNode -> next;     }
		if (isClient(nNode) && nNode != node) { return nNode;       }
		if (isClient(pNode) && pNode != node) { return pNode;       }
	}
	//If not returned by here must look for more nodes in the parent, recur
	if (node -> parent == NULL) { return NULL; } else {
		fprintf(stderr,"Calling get on %p\n", node -> parent);
		Node *j;
		j = getClosestNode(node -> parent);
		if (j != node) { return j; } else { return NULL; }
	}
}
