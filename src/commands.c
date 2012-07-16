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
	while ((token = nextToken(&request))) {
		tokens[i] = token;
		i++;
	}

	if (!strcmp(tokens[0], "dump")) {
		dumpTree();
	} else if (!strcmp(tokens[0], "layout")) {
		fprintf(stderr, "Setting layout to: %s", tokens[1]);
		if (!strcmp(tokens[1], "vertical"))
			focusedNode -> parent -> layout = VERTICAL; 
		else if (!strcmp(tokens[1], "horizontal"))
			focusedNode -> parent -> layout = HORIZONTAL; 
		else if (!strcmp(tokens[1], "grid"))
			focusedNode -> parent -> layout = GRID;
		else if (!strcmp(tokens[1], "max"))
			focusedNode -> parent -> layout = MAX;

		placeNode(focusedNode -> parent,
				(focusedNode -> parent) -> x,     (focusedNode -> parent) -> y,
				(focusedNode -> parent) -> width, (focusedNode -> parent) -> height);

	} else if (!strcmp(tokens[0], "focus")) {
		if (!strcmp(tokens[1], "cycle")) {
			if (!strcmp(tokens[2], "previous"))  cycleFocus(0);
			else if (!strcmp(tokens[2], "next")) cycleFocus(1);	

		} else if (!strcmp(tokens[1], "direction")) {
			if      (!strcmp(tokens[2], "left"))  directionFocus(0);
			else if (!strcmp(tokens[2], "up"))    directionFocus(1);
			else if (!strcmp(tokens[2], "right")) directionFocus(2);
			else if (!strcmp(tokens[2], "down"))  directionFocus(3);
		}

	} else if (!strcmp(tokens[0], "select")) {
		if (!strcmp(tokens[1], "parent")) {
			fprintf(stderr, "Selecting parent node\n");
			if (!selectedNode) {
				fprintf(stderr, "Selected Node is NULL\n");
				selectNode(focusedNode -> parent, True);
			} else if (selectedNode -> parent) {
					fprintf(stderr, "Selecting from Parent\n");
					selectNode(selectedNode -> parent, True);
			}
		} else if (!strcmp(tokens[1], "next")) {

		} else if (!strcmp(tokens[1], "previous")) {

		}

	} else if (!strcmp(tokens[0], "containerize")) {

		if (!strcmp(tokens[1], "client")) containerize();

	} else if (!strcmp(tokens[0], "view")) {

		if (!strcmp(tokens[1], "parent")) {
			if (viewNode -> parent) {
				unmapNode(viewNode);
				viewNode = viewNode -> parent;
				placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
				focusNode(focusedNode, NULL);
			}

		} else if (!strcmp(tokens[1], "child")) {
			if (focusedNode != viewNode) {
				Node *n = focusedNode;
				while (n && n -> parent != viewNode) n = n -> parent;
				if (!n) return;

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

/* If there is a selectedNode, updates focus
 * depending, there may be a new selectedNode & focusNode OR
 * just a new focusNode and no selectedNode */
void cycleFocus(int direction) {
	if (direction != 0 && direction != 1) return;
	Node * newSelect = NULL;

	Node * focusOrigin = selectedNode ?  selectedNode : focusedNode;
	Node * newFocus = NULL;
	//Loopback incase, 
	//TODO: Could use focused node starting to make faster instead of focusOrigin?
	if (direction == 0) {
		if (!(newFocus = focusOrigin -> previous)) {
			newFocus = focusOrigin;
			while (newFocus -> next != NULL)
				newFocus = newFocus -> next;
		}
	} else {
		if (!(newFocus = focusOrigin -> next))  {
			newFocus = focusOrigin -> parent -> child;
		}
	}

		//Alright we have a prvious, now figure out focus & select
		//First time around and were selecting a container!
		//Loop to find client to focus to
	if (newFocus) {
			if (!isClient(newFocus)) newSelect = newFocus;
			while (!isClient(newFocus))
				newFocus = (newFocus -> focus) ? 
					newFocus -> focus : newFocus -> child;
	}


	focusNode(newFocus, NULL);
	selectNode(newSelect, True);
}


void directionFocus(int direction) {
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
		if (focusedNode -> previous || focusedNode -> next) {
			Node *insertNode; int insertPosition;
			fprintf(stderr, "Containerizing, using some ref brother\n");
			if (focusedNode -> previous) {
				insertNode = focusedNode -> previous; insertPosition = 1;
			} else if (focusedNode -> next) {
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
		focusNode(newFocus, NULL);
		placeNode(viewNode, 
				viewNode -> x, viewNode -> y, 
				viewNode -> width, viewNode -> height);
	}
}
