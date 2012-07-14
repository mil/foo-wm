#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo-wm.h"
#include "commands.h"
#include "tree.h"
#include "util.h"


char * nextToken(char ** tokenString) {
	char *command;
	command = strsep(tokenString, " ");

	if (command == NULL) return NULL;

	char *newLine = strchr(command, '\n');
	if (newLine != NULL) 
		*newLine = '\0';

	return command;
}

void handleCommand(char* request) {
	fprintf(stderr, "Recv from FIFO: %s", request);

	char *tokens[5];

	char *token; int i = 0;
	while ((token = nextToken(&request)) != NULL) {
		tokens[i] = token;
		i++;
	}

	if (!strcmp(tokens[0], "dump")) {
		dumpTree();
	} else if (!strcmp(tokens[0], "layout")) {
		fprintf(stderr, "Setting layout to: %s", tokens[1]);
		if (!strcmp(tokens[1], "vertical"))
			focusedNode -> parent -> layout = 0; 
		else if (!strcmp(tokens[1], "horizontal"))
			focusedNode -> parent -> layout = 1; 
		else if (!strcmp(tokens[1], "grid"))
			focusedNode -> parent -> layout = 2;
		else if (!strcmp(tokens[1], "max"))
			focusedNode -> parent -> layout = 3;

		placeNode(focusedNode -> parent,
				(focusedNode -> parent) -> x,     (focusedNode -> parent) -> y,
				(focusedNode -> parent) -> width, (focusedNode -> parent) -> height);

	} else if (!strcmp(tokens[0], "focus")) {
		if      (!strcmp(tokens[1], "left"))  focus(0);
		else if (!strcmp(tokens[1], "up"))    focus(1);
		else if (!strcmp(tokens[1], "right")) focus(2);
		else if (!strcmp(tokens[1], "down"))  focus(3);

	} else if (!strcmp(tokens[0], "select")) {
		if (!strcmp(tokens[1], "parent")) {
			fprintf(stderr, "Selecting parent node\n");
			if (selectedNode == NULL) {
				fprintf(stderr, "Selected Node is NULL\n");
				selectNode(focusedNode -> parent, True);
			} else {
				if (selectedNode -> parent != NULL) {
					fprintf(stderr, "Selecting from Parent\n");
					selectNode(selectedNode -> parent, True);
				}
			}
		} else if (!strcmp(tokens[1], "next")) {

		} else if (!strcmp(tokens[1], "previous")) {

		}

	} else if (!strcmp(tokens[0], "containerize")) {

		if (!strcmp(tokens[1], "client")) containerize();

	} else if (!strcmp(tokens[0], "view")) {

		if (!strcmp(tokens[1], "parent")) {
			if (viewNode -> parent != NULL) {
				unmapNode(viewNode);
				viewNode = viewNode -> parent;
				placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
				focusNode(focusedNode);
			}

		} else if (!strcmp(tokens[1], "child")) {

			if (focusedNode != viewNode) {
				Node *n = focusedNode;
				while (n -> parent != viewNode && n != NULL) { n = n -> parent; }

				unmapNode(viewNode);
				viewNode = n;
				placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
			}
		}
	} else if (!strcmp(tokens[0], "kill")) {
		if (!strcmp(tokens[1], "client")) {
			kill();
		} else if (!strcmp(tokens[1], "container")) {
			dumpTree();
			fprintf(stderr, "Destroy Container %p\n", focusedNode -> parent);
			//destroyContainer(currentContainer);
			dumpTree();
		}
	}
}


void focus(int direction) {
	/*
	//Direction: 0 - Left, 1 - Up, 2 - Right, 3 - Down
	switch (focusedNode -> parent -> layout) {

	//Vertical Container
	case 0:
	switch (direction) {
	case 0: //Get client from parent to the left

	break;
	case 1: //Get the client upward
	break;

	case 2: //Get client from parent to the right
	break;

	case 3: //Get client downward
	break;
	}

	break;

	//Horizontal Container
	case 1: 
	switch (direction)  {
	case 0: //Get client to the left
	break;

	case 1: //Get client from container upwards
	break;

	case 2: //Get client to the right
	break;

	case 3: //Get client from container dowwards

	break;
	}
	break;
	}
	*/
}

void containerize() {

	Node * newContainer    = allocateNode();

	if (selectedNode != NULL) {
		if (selectedNode -> previous != NULL || selectedNode -> next != NULL) {
			Node *insertNode; int insertPosition;
			if (selectedNode -> previous != NULL) {
				insertNode = selectedNode -> previous; insertPosition = 1;
			} else if (focusedNode -> next != NULL) {
				insertNode = selectedNode -> next;     insertPosition = 0;
			}

			parentNode(selectedNode, newContainer);
			brotherNode(newContainer, insertNode, insertPosition);
			placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
		}


	} else if (focusedNode != NULL) { /* Working iwth a focused Client */
		/* Containerizing a client that is one of many in an existing container */
		if (focusedNode -> previous != NULL || focusedNode -> next != NULL) {
			Node *insertNode; int insertPosition;
			fprintf(stderr, "Containerizing, using some ref brother\n");
			if (focusedNode -> previous != NULL) {
				insertNode = focusedNode -> previous; insertPosition = 1;
			} else if (focusedNode -> next != NULL) {
				insertNode = focusedNode -> next;     insertPosition = 0;
			} else {
				fprintf(stderr, "NO INSERT NODE\n");
			}

			parentNode(focusedNode, newContainer);
			brotherNode(newContainer, insertNode, insertPosition);

			placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);

		} else { /* Containerizing client that is alone in container */

		}
	} else {
		//Defaulted with just a view node
	}
}

void kill() {
	dumpTree();
	fprintf(stderr, "Destroying Client %p\n", focusedNode);

	if (isClient(focusedNode)) {

		/* Save closest client and destroy node */
		Node *newFocus = getClosestClient(focusedNode);

		if (focusedNode == viewNode) { viewNode = viewNode -> parent; }

		if (
				focusedNode -> next == NULL && focusedNode -> previous == NULL
				&& viewNode == focusedNode -> parent) {
			if (focusedNode -> parent -> parent != NULL) {
				fprintf(stderr, "Parent's parent exists\n");
				viewNode = focusedNode -> parent -> parent;
			} else {
				fprintf(stderr, "Parent's parent does not exist\n");
				viewNode = focusedNode -> parent;
			}
		}

		destroyNode(focusedNode);

		dumpTree();

		/* Give the closeset client of destroyed node focus and rerender */
		focusNode(newFocus);
		placeNode(viewNode, 
				viewNode -> x, viewNode -> y, 
				viewNode -> width, viewNode -> height);
	}
}




