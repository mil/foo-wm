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

		Node *setNode = selectedNode ? selectedNode : focusedNode -> parent;
		if (!strcmp(tokens[1], "vertical"))
			setNode -> layout = VERTICAL; 
		else if (!strcmp(tokens[1], "horizontal"))
			setNode -> layout = HORIZONTAL; 
		else if (!strcmp(tokens[1], "grid"))
			setNode -> layout = GRID;
		else if (!strcmp(tokens[1], "max"))
			setNode -> layout = MAX;

		placeNode(setNode, 
				setNode -> x, setNode -> y, setNode -> width, setNode -> height);

	} else if (!strcmp(tokens[0], "focus")) {
		if (!strcmp(tokens[1], "cycle")) {
			if (!strcmp(tokens[2], "previous"))  cycleFocus(PREVIOUS);
			else if (!strcmp(tokens[2], "next")) cycleFocus(NEXT);	

		} else if (!strcmp(tokens[1], "direction")) {
			if      (!strcmp(tokens[2], "left"))  directionFocus(LEFT);
			else if (!strcmp(tokens[2], "up"))    directionFocus(UP);
			else if (!strcmp(tokens[2], "right")) directionFocus(RIGHT);
			else if (!strcmp(tokens[2], "down"))  directionFocus(DOWN);
		}

	} else if (!strcmp(tokens[0], "select")) {
		if (!strcmp(tokens[1], "parent")) {
			fprintf(stderr, "Selecting parent node\n");
			if (!selectedNode) {
				fprintf(stderr, "Selected Node is NULL\n");
				if (focusedNode && focusedNode -> parent)
					selectNode(focusedNode -> parent, True);
			} else if (selectedNode -> parent) {
					fprintf(stderr, "Selecting from Parent\n");
					selectNode(selectedNode -> parent, True);
			}
		} else if (!strcmp(tokens[1], "child")) {
			fprintf(stderr, "Selectign on child node\n");

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
	if (direction != PREVIOUS && direction != NEXT) return;
	Node * newSelect = NULL;

	Node * focusOrigin = selectedNode ?  selectedNode : focusedNode;
	Node * newFocus = NULL;
	//Loopback incase, 
	//TODO: Could use focused node starting to make faster instead of focusOrigin?
	if (direction == PREVIOUS) {
		if (!(newFocus = focusOrigin -> previous)) {
			newFocus = focusOrigin;
			while (newFocus -> next)
				newFocus = newFocus -> next;
		}
	} else {
		if (!(newFocus = focusOrigin -> next) && focusOrigin -> parent)  {
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

	if (selectedNode) {
		if (selectedNode -> previous || selectedNode -> next) {
			Node *insertNode; int insertPosition;
			if (selectedNode -> previous) {
				insertNode = selectedNode -> previous; insertPosition = NEXT;
			} else if (selectedNode -> next) {
				insertNode = selectedNode -> next;     insertPosition = PREVIOUS;
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
				insertNode = focusedNode -> previous; insertPosition = NEXT;
			} else if (focusedNode -> next) {
				insertNode = focusedNode -> next;     insertPosition = PREVIOUS;
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

		if (focusedNode == viewNode) viewNode = viewNode -> parent;

		if ( !focusedNode -> next && !focusedNode -> previous 
				&& focusedNode -> parent) {
			viewNode = focusedNode -> parent -> parent ?
				focusedNode -> parent -> parent : focusedNode -> parent;
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
