#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "fifo-wm.h"
#include "tree.h"
#include "util.h"
#include "window.h"

void crawlNode(Node * node, int level) {
	int j; for (j = level; j > 0; j--) { fprintf(stderr, "|\t"); }

	if (isClient(node)) {
		fprintf(stderr, "Client (%p)", node);
		if (node == activeNode) { fprintf(stderr, " [FOCUS]"); }
		fprintf(stderr, "\n");

	} else {
		char *label;
		switch (node -> layout) {
			case 0: label = "Vertical"; break;
			case 1: label = "Horizontal"; break;
			case 2: label = "Grid"; break;
		}
		fprintf(stderr, "Container (%p) %s\n", node, label);

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
	centerPointer(&n -> window);
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

	focusNode(getClosestClient(node));

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


void brotherNode(Node *node, Node * brother, int position) {
	if (position == 0) {
		node -> next = brother;
		if (brother -> previous == NULL) { //Pop in the front
			node -> parent = brother -> parent;
			node -> parent -> child = node;
		} else {
			//Shift previous pointer
			node -> previous = brother -> previous;
			brother -> previous = node;
		}
	} else if (position == 1) {
		node -> previous = brother;
		node -> next = brother -> next;
		brother -> next = node;
	}
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
				(x < 0) ? 0 : x, 
				(y < 0) ? 0 : y, 
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

		/* Determine the number of rows and cols */
		int rows; int cols;
		switch (node -> layout) {
			case 0: cols = 1; rows = children; break;
			case 1: cols = children; rows = 1; break;
			case 2: gridDimensions(children, &rows, &cols); break;
		}


		for (a = node -> child; a != NULL; a = a -> next, i++) {
			a -> x = x + (i % cols) * (width/cols) + padding;
			a -> y = y + ((int)(i / cols)) * (height/rows) + padding;
			a -> width = width / cols - (padding * 2);
			a -> height = height / rows - (padding * 2);

			if (node -> layout == 2) {
				//Two nodes, edge case for formula
				if (children == 2) { a -> height = height - (padding * 2); }
				//Scretch the last child
				if (i + 1 == children) { a -> width = x + width - a -> x - (padding * 2); }
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

/* Gets the next brother client to node, in given direction 
 * [Container] - [Client X] - [Container] - [Container] - [Client Y]
 * Given Client X, function would loop until hitting Client Y
 * */
Node * getBrotherClient(Node * node, int direction) {
	Node *pNode = node;
	Node *nNode = node;

	while (pNode -> previous != NULL || nNode -> next != NULL) {
		if (pNode -> previous != NULL) { pNode = pNode -> previous; }
		if (nNode -> next != NULL    ) { nNode = nNode -> next;     }
		switch (direction) {
			case 0:
				if (isClient(pNode) && pNode != node) { return pNode; }
				if (isClient(nNode) && nNode != node) { return nNode; }
				break;
			case 1:
				if (isClient(nNode) && nNode != node) { return nNode; }
				if (isClient(pNode) && pNode != node) { return pNode; }
				break;
		}
	}
	return NULL;
}

Node * getClosestClient(Node * node) {
	Node * returnNode = NULL;
	Node * currentNode = node;

	/* Calls getBrotherClient going up the tree until a client is found */
	while (returnNode == NULL) {
		returnNode = getBrotherClient(currentNode, 1);
		if (returnNode == NULL) {
			if (currentNode -> parent != NULL) { //Try one level up
				currentNode = currentNode -> parent;
			} else { //We didn't find a brother and we have no parent, end game
				return NULL;
			}
		} else {  //We found a client 
			return returnNode; 
		}
	}
	return NULL;
}
