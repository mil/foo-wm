#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "foo-wm.h"
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
Bool unfocusNode(Node * n, Bool focusPath) {
	if (!n) return False;

	Bool setView = (n == viewNode) ? True : False;
	fprintf(stderr, "Yo i be unfocusing %p\n", n);

	//Unfocusing Code for previous focusedNode
	if (isClient(n)) {
		/*XSetWindowBorder(display, n -> window, 
			(n -> parent -> focus == n) ? 
inactiveFocusedColor : inactiveUnfocusedColor);
*/

		//This should only apply to the most innard focus of focusedNode, follow ptrs
		if (focusPath)
			XGrabButton(display, AnyButton, AnyModifier,
					n -> window, True, ButtonPressMask | ButtonReleaseMask,
					GrabModeAsync, GrabModeAsync, None, None);

	} else {
		//Recursive loop on children to set 

		Node *c;
		for (c = n -> child; c; c = c -> next)
			unfocusNode(c, c -> parent -> focus == c ? True : False);
	}

	return setView;
}



//This should focus OR select
void focusNode(Node * n, XEvent * event, Bool setFocused, Bool focusPath) {
	if (!n || n == focusedNode) return;
	fprintf(stderr, "Focusing %p", n);

	//Bool setView = False; //Wether the viewNode needs to be moved
	if (focusPath && setFocused) { 
		fprintf(stderr, "\n\nNode %p, is in the focus patho\n\n", n);
		unfocusNode(focusedNode, True);
		if (setFocused && n -> parent)   n -> parent -> focus = n;
	}

	if (setFocused)  focusedNode = n;
	//if (setView)     viewNode    = n;

	// Are we at the bottom level	
	if (isClient(n)) {
		if (n -> parent)  {
			placeNode(n -> parent, n -> parent -> x, n -> parent -> y,
					n -> parent -> width, n -> parent -> height);
		} 

		if (focusPath) {
				XSetInputFocus(display, n -> window, RevertToParent, CurrentTime);	
				XUngrabButton(display, AnyButton, AnyModifier, n ->window);
				XRaiseWindow(display, n -> window);

			if (event) {
				// Set the Input focus, and ungrab the window (no longer point to click)
				XSendEvent(display, n -> window, True, ButtonPressMask, event);
			} else {
				fprintf(stderr, "YO I AM CENTERING POINT on %p !\n\n", n);
				centerPointer(&n -> window);
			}
		}


	} else {
		fprintf(stderr, "focus called on a container");
		Node *i; for (i = n -> child; i; i = i -> next) {
			focusNode(i, NULL, False, //Recall Focusnode
					i -> parent -> focus == i ? True : False);
		}
	}

	//if (setView) placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
}


void destroyNode(Node * n) {
	if (!n) return;

	//Recursvily unmap up any lone parents
	if (n -> parent && n -> parent != viewNode && isOnlyChild(n) && 
			n -> parent -> child == n && n -> parent -> parent) {
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
	if (!(node && node -> parent)) return;
	fprintf(stderr, "Unparent called\n");

	//Move parent's child pointer if were it....
	if (node -> parent -> child == node) 
		node -> parent -> child = node -> next;

	//Move the next and previous pointers to cut out the node
	if (node -> next)     node -> next -> previous = node -> previous;
	if (node -> previous) node -> previous -> next = node -> next;

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
		if (node -> next) node -> next -> previous = node;
		brother -> next = node;
	}
}


void parentNode(Node *node, Node *parent) {
	fprintf(stderr, "Pareting node %p into parent %p\n", node, parent);
	if (!node || !parent) return;  //Cant add to NULL

	unparentNode(node); //Unparent then set the parent to new parent
	node -> parent = parent;
	if (!parent -> focus) parent -> focus = node;

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

		Node *b = node; Bool inFocusPath = False;

		if (b == focusedNode) {
			inFocusPath = True;
		} else {
			do {  //Figure out if were in the focus path
				b = b -> parent;
				if (b == focusedNode) inFocusPath = True;
			} while (b -> parent);
		}
		fprintf(stderr, "Determining color for %p", node);
		if (inFocusPath) {
			if (focusedNode == node) {
				XSetWindowBorder(display, node -> window, activeFocusedColor);
			} else {
				if (node -> parent -> focus == node) {
					XSetWindowBorder(display, node -> window, inactiveFocusedColor);
				} else {
					XSetWindowBorder(display, node -> window, activeUnfocusedColor);
				}
			}
		} else {
			if (node -> parent -> focus == node) {
				XSetWindowBorder(display, node -> window, inactiveFocusedColor);
			} else {
				XSetWindowBorder(display, node -> window, inactiveUnfocusedColor);
			}
		}

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


Bool isClient(Node * node) { /* Is the node a client? */
	if (node && (node -> window != (Window) NULL)) return True;
	else return False;	
}

Bool isOnlyChild(Node * node) { /* Is the node an only child */
	if (node && (node -> next || node -> previous)) return False;
	else return True;
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

/* Gets brother in specific direction 
 * If can't get brother in direction, returns NULL
 * */
Node * getBrotherByDirection(Node * node, int direction) {
	if (!node) return NULL;

	// In Max layout directional focus aint a work
	if (node -> parent && node -> parent -> layout == MAX)
		return NULL;

	// Count up children and find us in our parent
	int parentChildren = 0, nodePosition = -1;
	Node *n = NULL;
	for (n = node -> parent -> child; n; n = n -> next) {
		parentChildren++;
		if (n == node) nodePosition = parentChildren;
	}


	// Get the dimensions of the current grid
	int rows = 0, cols = 0;
	gridDimensions(parentChildren, &rows, &cols);

	int count, dest;
	//Determine the brother
	switch (direction) {
		case LEFT:
			if (((nodePosition - 1) % rows != 0) && node -> previous)
				return node -> previous;
			break;
		case RIGHT:
			if ((nodePosition % rows != 0) && node -> next)
				return node -> next;
			break;
		case UP:
			dest = nodePosition - cols;
			if (dest >= 0) {
				n = node;
				while (dest != nodePosition) {
					n = n -> previous;
					dest++;
				}
				return n;
			}	else { return NULL; }
			break;
		case DOWN:
			dest = nodePosition + cols;
			if (dest <= parentChildren) {
				n = node;
				while (dest != nodePosition) {
					n = n -> next;
					dest--;
				}
				return n;
			} else { return NULL; }
			break;
	}

	return NULL;
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
	temp = a -> previous; a -> previous = b -> previous;
	if (a -> previous) a -> previous -> next = a;
	b -> previous = temp;
	if (b -> previous) b -> previous -> next = b;

	/* Update Next Pointer */
	temp = a -> next; a -> next = b -> next;
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
