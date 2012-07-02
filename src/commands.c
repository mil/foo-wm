#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo-wm.h"
#include "commands.h"
#include "tree.h"


void focus(int direction) {
	//Direction: 0 - Previous, 1 - Next, 2 - Parent
	/*
	if (direction == 0) {
		if ((currentContainer -> focus) -> next != NULL) {
			focusClient((currentContainer -> focus) -> next);
		} else if ((currentContainer -> focus) -> previous != NULL) {
			focusClient((currentContainer -> focus) -> previous);
		}
	} else if (direction == 1) {
		if ((currentContainer -> focus) -> previous != NULL) {
			focusClient((currentContainer -> focus) -> previous);
		} else if ((currentContainer -> focus) -> next != NULL) {
			focusClient((currentContainer -> focus) -> next);
		}
	} else if (direction == 2) {
		if (currentContainer -> parent != NULL) {
			currentContainer = currentContainer -> parent;
		} else {
			Container * newContainer = malloc(sizeof(Container));
			parentContainer(currentContainer, newContainer);
			currentContainer = newContainer;
			placeContainer(currentContainer, 
				currentContainer -> x, currentContainer -> y, 
				currentContainer -> width, currentContainer -> height);
		}
	}
	*/
}

void handleCommand(char* request) {
	fprintf(stderr, "Recv from FIFO: %s", request);

	//Array to contain pointers to tokens, max of 5 tokens
	char *tokens[5];

	int counter = 0;
	char *lastToken, *token;
	for ( token = strtok_r(request, " ", &lastToken); token; token = strtok_r(NULL, " ", &lastToken)) {
		tokens[counter++] = token;
		fprintf(stderr, "Adding Token %s\n", token);
	}

	if (!strcmp(tokens[0], "dump")) {
		dumpTree();
	} else if (!strcmp(tokens[0], "layout")) {
		fprintf(stderr, "Setting layout to: %s", tokens[1]);
		if (!strcmp(tokens[1], "vertical")) {	activeNode -> layout = 0; 
		} else if (!strcmp(tokens[1], "horizontal")) { activeNode -> layout = 1; }
		placeNode(activeNode,
				activeNode -> x,     activeNode -> y,
				activeNode -> width, activeNode -> height);

	} else if (!strcmp(tokens[0], "focus")) {
		if (!strcmp(tokens[1], "next"))
			focus(0);
		else if (!strcmp(tokens[1], "previous"))
			focus(1);
		else if (!strcmp(tokens[1], "parent"))
			focus(2);

	} else if (!strcmp(tokens[0], "containerize")) {
		if ((activeNode -> focus) -> previous != NULL) {
			fprintf(stderr,"Containerizing");
			Node * newContainer = malloc(sizeof(Node));	
			parentNode(activeNode -> focus, newContainer);
			parentNode(newContainer, activeNode);
			activeNode = newContainer;
			placeNode(viewNode, 0, 0,
					DisplayWidth(display, activeScreen),
					DisplayHeight(display, activeScreen));
		}	else {
			fprintf(stderr, "Containerize called but alone in contianer");
		}

	} else if (!strcmp(tokens[0], "view")) {
		if (!strcmp(tokens[1], "parent")) {
			if (viewNode -> parent != NULL) {
				unmapNode(viewNode);
				viewNode = viewNode -> parent;
				placeNode(viewNode, 0, 0,
						DisplayWidth(display, activeScreen),
						DisplayHeight(display, activeScreen));
			}

		} else if (!strcmp(tokens[1], "child")) {
			viewNode = activeNode;
			placeNode(viewNode, 0, 0,
					DisplayWidth(display, activeScreen),
					DisplayHeight(display, activeScreen));
		}
	} else if (!strcmp(tokens[0], "kill")) {
		/*
		if (!strcmp(tokens[1], "client")) {


			dumpTree();
			fprintf(stderr, "Destroy Client %p\n", currentContainer -> focus);

			destroyClient(currentContainer -> focus);

			dumpTree();

			placeContainer(currentContainer, 
					currentContainer -> x, currentContainer -> y, 
					currentContainer -> width, currentContainer -> height);
		} else if (!strcmp(tokens[1], "container")) {
			dumpTree();
			fprintf(stderr, "Destroy Container %p\n", currentContainer);
			destroyContainer(currentContainer);
			dumpTree();
		}

		*/
	}
}
