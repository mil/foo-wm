#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "fifo-wm.h"
#include "tree.h"
#include "lookup.h"
#include "util.h"
#include "window.h"

void crawlNode(Node * node, int level) {
	int j; for (j = level; j > 0; j--) { fprintf(stderr, "|\t"); }

	if (isClient(node)) {
		fprintf(stderr, "Client (%p)", node);
		if (node == focusedNode) fprintf(stderr, " [Focused]");
		if (node == viewNode)    fprintf(stderr, " [View]");
		//fprintf(stderr, " || N[%p] P[%p]", node -> next, node -> previous);
		fprintf(stderr, "\n");

	} else {
		char *label;
		switch (node -> layout) {
			case 0: label = "Vertical"; break;
			case 1: label = "Horizontal"; break;
			case 2: label = "Grid"; break;
			case 3: label = "Max"; break;
		}

		fprintf(stderr, "Container (%p) %s", node, label);
		if (node == selectedNode) fprintf(stderr, " [Selected]");
		if (node == viewNode)     fprintf(stderr, " [View]");
		//fprintf(stderr, " || N[%p] P[%p]", node -> next, node -> previous);
		fprintf(stderr, "\n");

		Node *n;
		for (n = node -> child; n != NULL; n = n -> next) {
			crawlNode(n, level + 1);	
		}
	}

}

void dumpTree() {
	fprintf(stderr, "Printing the tree\n");
	fprintf(stderr, "----------------------------------\n");
	crawlNode(rootNode, 0);
	fprintf(stderr, "----------------------------------\n");
}

//Will only work on nodes with windows for now
void focusNode(Node * n) {
	if (focusedNode == n) return;
	if (!isClient(n)) { 
		fprintf(stderr, "Trying to focus a node thats not a client !\n"); 
		return;
	}

	/* Regrab the focused window (for pointer to click)
	 * as it will no longer be focused */
	if (focusedNode != NULL) {
		XGrabButton(display, AnyButton, AnyModifier,
				focusedNode -> window, True,
				ButtonPressMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None);
	}

	if (isClient(focusedNode))
		XSetWindowBorder(display, focusedNode -> window, unfocusedColor);

	selectedNode = NULL;
	focusedNode = n;
	centerPointer(&n -> window);

	/* Set the Input focus, and ungrab the window (no longer point to click */
	XSetInputFocus(display, n -> window, RevertToParent, CurrentTime);	
  XUngrabButton(display, AnyButton, AnyModifier, focusedNode->window);
	XRaiseWindow(display, n -> window);


}

void selectNode(Node * n, Bool setSelected) {
	if (setSelected == True) selectedNode = n;

	Node *i;
	for (i = n -> child; i != NULL; i = i -> next) {
		if (isClient(i) == True) {
			fprintf(stderr, "Node %p", n);
			XSetWindowBorder(display, i->window, focusedColor);	
		} else {
			selectNode(i, False);
		}
	}
}


void destroyNode(Node * n) {
	fprintf(stderr, "PRE DESTROY\n");
	dumpTree();

	if (n == NULL) return;
	if (n -> parent == NULL) return;

	//Asking to destroy a node who once disowned would leave an empty node
	//Recall destroyNode on parent
	if ( n -> next == NULL     && n -> parent -> child == n && 
			n -> previous == NULL && n -> parent -> parent != NULL) {
		fprintf(stderr, "Calling destroy node on parent\n");
		destroyNode(n -> parent);
		return;
	}

	//Unparent the node
	unparentNode(n);

	//Recursivly unmap all children of the node
	if (isClient(n)) {
		removeLookupEntry(&n -> window);
		XDestroyWindow(display, n -> window);
	} else {
		fprintf(stderr, "The child is %p\n", n -> child);
		if (n -> child  -> next != NULL) {
			Node * child;
			for (child = n -> child; child != NULL; child = child -> next) {
				destroyNode(child);
			}
		}

	}

	if (n == focusedNode) focusedNode = NULL;
	free(n);
}


void unparentNode(Node *node) {
	if (node == NULL || node -> parent == NULL) return;

	fprintf(stderr, "unparent called");

	//Move parent's child pointer if were it....
	if (node -> parent -> child == node) 
		node -> parent -> child = node -> next;

	//Move the next and previous pointers to cut out the node
	if (node -> next != NULL)     node -> next -> previous = node -> previous;
	if (node -> previous != NULL) node -> previous -> next = node -> next;

	//Set our parent to NULL
	node -> parent = NULL; node -> next = NULL; node -> previous = NULL;
}


void brotherNode(Node *node, Node * brother, int position) {
	node -> parent = brother -> parent;

	if (position == 0) {
		node -> next = brother;
		if (brother -> previous == NULL) { //Pop in the front
			node -> parent -> child = node;
		} else {
			//Shift previous pointer
			node -> previous = brother -> previous;
			brother -> previous = node;
		}
	} else if (position == 1) {
		node -> previous = brother;
		node -> next = brother -> next;
		if (node -> next != NULL) node -> next -> previous = node;
		brother -> next = node;
	}
}


void parentNode(Node *node, Node *parent) {
	fprintf(stderr, "Pareting node %p into parent %p\n", node, parent);
	if (parent == NULL) { return; } //Cant add to NULL
	fprintf(stderr, "Made it here\n");

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
	if (isClient(node)) {
		XUnmapWindow(display, node -> window);
	} else {
		Node *n;
		for (n = node -> child; n != NULL; n = n -> next)
			unmapNode(n);
	}
}

void placeNode(Node * node, int x, int y, int width, int height) {
	if (node == NULL) return;
	node -> x = x; node -> y = y;
	node -> width = width; node -> height = height;
	fprintf(stderr, "Place Node XY:[%d, %d], WH:[%d, %d]\n", x, y, width, height);

	if (isClient(node)) {
		fprintf(stderr,"Rendering window\n");
		XMapWindow(display, node -> window);
		XRaiseWindow(display, node -> window);

		XMoveResizeWindow(display, node -> window, 
				(x < 0) ? 0 : x, 
				(y < 0) ? 0 : y, 
				width - (border*2), 
				height - (border *2));
		XSetWindowBorderWidth(display, node -> window, border);
		if (focusedNode == node) {
			XSetWindowBorder(display, node -> window, focusedColor);
		} else {
			XSetWindowBorder(display, node -> window, unfocusedColor);
		}

	} else {
		//Count up children prior to loop
		int children = 0; int i = 0; Node *a;
		if (node -> child == NULL) return;
		for (a = node -> child; a != NULL; a = a -> next) { children++; }

		/* Determine the number of rows and cols */
		int rows; int cols;
		switch (node -> layout) {
			case 0: cols = children; rows = 1; break;
			case 1: cols = 1; rows = children; break;
			case 2: gridDimensions(children, &rows, &cols); break;
			case 3: cols = 1; rows = 1; break;
		}

		Bool callPlace;
		int pad;
		for (a = node -> child; a != NULL; a = a -> next, i++) {
			if (isClient(a)) pad = clientPadding;
			else pad = containerPadding;

			callPlace = True;
			if (node -> layout == 3) {
				if (focusedNode == a) i = 0; 
				else callPlace = False;
			}

			if (callPlace != False) {	
				a -> x = x + (i % cols) * (width/cols) + pad;
				a -> y = y + ((int)(i / cols)) * (height/rows) + pad;
				a -> width = width / cols - (pad * 2);
				a -> height = height / rows - (pad * 2);

				if (node -> layout == 2) {
					//Two nodes, edge case for formula
					if (children == 2)      a -> height = height - (pad * 2);
					//Stretch the last child
					if (i + 1 == children)  a -> width = x + width - a -> x - (pad * 2);
				}
				placeNode(a, a -> x, a -> y, a -> width, a -> height);
			}
		}
	}
}


Bool isClient(Node * node) {
	if (node == NULL)                    return False;
	if (node -> window != (Window) NULL) return True;
	return False;	
}

/* Gets the next brother client to node, in given direction 
 * [Container] - [Client X] - [Container] - [Container] - [Client Y]
 * Given Client X, function would loop until hitting Client Y
 * */
Node * getBrotherClient(Node * node, int direction) {
	Node *pNode = node;
	Node *nNode = node;

	while (pNode -> previous != NULL || nNode -> next != NULL) {
		if (pNode -> previous != NULL) pNode = pNode -> previous;
		if (nNode -> next != NULL    ) nNode = nNode -> next;
		switch (direction) {
			case 0:
				if (isClient(pNode) && pNode != node) return pNode;
				if (isClient(nNode) && nNode != node) return nNode;
				break;
			case 1:
				if (isClient(nNode) && nNode != node) return nNode;
				if (isClient(pNode) && pNode != node) return pNode;
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
			if (currentNode -> parent != NULL) currentNode = currentNode -> parent;
			else                               return NULL;
		} else {  //We found a client 
			return returnNode; 
		}
	}
	return NULL;
}
