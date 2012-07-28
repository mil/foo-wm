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

		fprintf(stderr, "Container (%p) %s (focus=%p)", node, label, node -> focus);
		if (node == focusedNode)  fprintf(stderr, " [Focused]");
		if (node == viewNode)     fprintf(stderr, " [View]");
		//fprintf(stderr, " || N[%p] P[%p]", node -> next, node -> previous);
		fprintf(stderr, "\n");

		Node *n;
		for (n = node -> child; n; n = n -> next)
			crawlNode(n, level + 1);	
	}

}

void dumpTree() {
	fprintf(stderr, "Printing the tree\n");
	fprintf(stderr, "----------------------------------\n");
	crawlNode(rootNode, 0);
	fprintf(stderr, "----------------------------------\n");
}



/* Unfocuses the currently focused node, called only by focusNode 
 * Returns Bool if an update of the view is needed
 * Dangerous if called alone */
Bool unfocusNode(Node * n) {
	if (!n) return False;

	Bool setView = (n == viewNode) ? True : False;
	fprintf(stderr, "Yo i be unfocusing\n");

	//Unfocusing Code for previous focusedNode
	if (isClient(n)) {
		XSetWindowBorder(display, n -> window, unfocusedColor);

		//This should only apply to the most innard focus of focusedNode, follow ptrs
		XGrabButton(display, AnyButton, AnyModifier,
				n -> window, True,
				ButtonPressMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None);

	} else {
		//Recursive loop on children to set 

	}

	return setView;
}



//This should focus OR select
void focusNode(Node * n, XEvent * event, Bool setFocused) {
	if (!n || n == focusedNode) return;
	fprintf(stderr, "Focusing %p", n);


	if (setFocused)             focusedNode = n;
	if (n -> parent)            n -> parent -> focus = n;

	if (isClient(n)) {
		if (n -> parent)  {
			n -> parent -> focus = n;
			//if (focusedNode -> parent -> layout == MAX)
			placeNode(n -> parent, n -> parent -> x, n -> parent -> y,
					n -> parent -> width, n -> parent -> height);
		} 

		// Set the Input focus, and ungrab the window (no longer point to click
		XSetInputFocus(display, n -> window, RevertToParent, CurrentTime);	
		XUngrabButton(display, AnyButton, AnyModifier, n ->window);
		XSetWindowBorder(display, n -> window, focusedColor);
		XRaiseWindow(display, n -> window);

		//Passed an event
		if (event) {
			fprintf(stderr, "Focuing a node with an event\n");
			XSendEvent(display, n -> window, True, ButtonPressMask, event);
		} else {
			fprintf(stderr, "Focuing a node without an event\n");
			centerPointer(&n -> window);
		}
	} else {
		fprintf(stderr, "focus claled ona  ctaonerin");
		Node *i;
		for (i = n -> child; i; i = i -> next) {
			if (i == n) { //The Focused node requested thats a container
				focusNode(n, NULL, False);
			} else if (isClient(i)) { //Must dehigh these clients, unfocused_inactive
				XSetWindowBorder(display, i->window, selectedColor);	
			} else {
				//We have a container as a child of the focusedNode BUT not in the path 
				//to focusing... ned a function to set unfocused_inactive OR in placeNode
			}
		}
	}
}
void destroyNode(Node * n) {
	if (!n) return;

	//Recursvily unmap up any lone parents
	if ( n -> parent && !n -> next  &&  !n -> previous && 
			n -> parent -> child == n && n -> parent -> parent
			&& n -> parent != viewNode) {
		destroyNode(n -> parent);
		return;
	}

	//Unparent the node
	unparentNode(n);

	//Recursivly unmap down all children of the node
	if (isClient(n)) {
		removeLookupEntry(&n -> window);
		XDestroyWindow(display, n -> window);
	} else {
		Node *destroy = n -> child; Node *next = NULL;
		do {
			next = destroy -> next;
			destroyNode(destroy);
		} while (next);

	}

	//Set Focused Node if we just destroyed the focus, and free 
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
		if (!brother -> previous) { //Pop in the front
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
	if (!node || !parent) return;  //Cant add to NULL

	unparentNode(node); //Unparent then set the parent to new parent
	node -> parent = parent;

	//Find last in children of parent, add to end
	if (parent -> child) {
		Node *n = parent -> child;
		while (n -> next) n = n -> next;
		node -> previous = n;
		n -> next = node;
	} else {
		parent -> child = node;
	}
}


void unmapNode(Node * node) {
	if (isClient(node)) {
		XUnmapWindow(display, node -> window);
	} else {
		Node *n;
		for (n = node -> child; n; n = n -> next)
			unmapNode(n);
	}
}

void placeNode(Node * node, int x, int y, int width, int height) {
	if (!node) return;
	node -> x = x; node -> y = y; node -> width = width; node -> height = height;
	fprintf(stderr, "Place Node XY:[%d, %d], WH:[%d, %d]\n", x, y, width, height);

	if (isClient(node)) {
		fprintf(stderr,"Rendering window\n");
		XMapWindow(display, node -> window);
		XRaiseWindow(display, node -> window);
		XMoveResizeWindow(display, node -> window, 
				(x < 0) ? 0 : x, 
				(y < 0) ? 0 : y, 
				(width -  (border * 2)) > 0 ? (width - border * 2) : 1, 
				(height - (border * 2)) > 0 ? (height- border * 2) : 1);
		XSetWindowBorderWidth(display, node -> window, border);

		XSetWindowBorder(display, node -> window, 
				(focusedNode == node ? focusedColor : unfocusedColor));

	} else {
		//Count up children prior to loop
		int children = 0; int i = 0; Node *a = NULL;
		if (!node -> child) return;
		for (a = node -> child; a; a = a -> next) children++;

		/* Determine the number of rows and cols */
		int rows; int cols;
		switch (node -> layout) {
			case VERTICAL  : cols = children; rows = 1; break;
			case HORIZONTAL: cols = 1; rows = children; break;
			case GRID      : gridDimensions(children, &rows, &cols); break;
			case MAX       : cols = 1; rows = 1; break;
		}

		Bool callPlace;
		int pad;
		for (a = node -> child; a; a = a -> next, i++) {
			if (isClient(a)) pad = clientPadding;
			else pad = containerPadding;

			callPlace = True;
			if (node -> layout == MAX) {
				if (a -> parent -> focus == a) i = 0; 
				else callPlace = False;
			}

			a -> x = x + (i % cols) * (width/cols) + pad;
			a -> y = y + ((int)(i / cols)) * (height/rows) + pad;
			a -> width = width / cols - (pad * 2);
			a -> height = height / rows - (pad * 2);

			if (callPlace) {	

				if (node -> layout == GRID) {
					//Two nodes, edge case for formula
					if (children == 2)      a -> height = height - (pad * 2);
					//Stretch the last child
					if (i + 1 == children)  a -> width = x + width - a -> x - (pad * 2);
				}
				placeNode(a, a -> x, a -> y, a -> width, a -> height);
			} else {
				fprintf(stderr, "Going to call unmap on %p\n", a);
				unmapNode(a);
			}
		}
	}
}


Bool isClient(Node * node) {
	if (!node) return False;
	if (node -> window != (Window) NULL) return True;
	return False;	
}


Node * getBrother(Node * node, int delta) {
	fprintf(stderr, "Getting the brother node");

	while (delta > 0) {
		if (node -> next)
			node = node -> next;
		else if (node -> parent && node -> parent -> child)
			node = node -> parent -> child;
		delta--;
	}

	while (delta < 0) {
		if (node -> previous) {
			node = node -> previous;
		} else if (node -> parent && node -> parent -> child) {
			node = node -> parent -> child;
			while (node -> next)
				node = node -> next;
		} else { fprintf(stderr, "Not a good situation\n"); }

		delta++;
	}

	return node;
}

/* Swaps nodes within the same container of the tree 
 * [ NULL <- A <-> B <-> C <-> D -> NULL ] */
void swapNodes(Node * a, Node * b) {
	if (!a || !b || a == b) return;


	/* First child / start of linked list */
	Node *temp = NULL;
	if (a -> parent -> child == a)      a -> parent -> child = b;
	else if (b -> parent -> child == b) b -> parent -> child = a;

	/* Update Previous Pointer */
	temp = a -> previous;
	a -> previous = b -> previous;
	if (a -> previous) a -> previous -> next = a;
	b -> previous = temp;
	if (b -> previous) b -> previous -> next = b;

	/* Update Next Pointer */
	temp = a -> next;
	a -> next = b -> next;
	if (a -> next) a -> next -> previous = a;
	b -> next = temp;
	if (b -> next) b -> next -> previous = b;


	/* Replace node */
	placeNode(viewNode, viewNode -> x, viewNode -> y,
			viewNode -> width, viewNode -> height);
}


/* Gets the next brother client to node, in given direction 
 * [Container] - [Client X] - [Container] - [Container] - [Client Y]
 * Given Client X, function would loop until hitting Client Y
 * */
Node * getBrotherClient(Node * node, int direction) {
	Node *pNode = node;
	Node *nNode = node;

	while (pNode -> previous || nNode -> next) {
		if (pNode -> previous ) pNode = pNode -> previous;
		if (nNode -> next     ) nNode = nNode -> next;
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
	while (!returnNode) {
		returnNode = getBrotherClient(currentNode, 1);
		if (!returnNode) {
			if (currentNode -> parent) currentNode = currentNode -> parent;
			else                               return NULL;
		} else {  //We found a client 
			return returnNode; 
		}
	}
	return NULL;
}
