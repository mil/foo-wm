#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo-wm.h"
#include "commands.h"
#include "tree.h"
#include "util.h"


void focus(int direction) {
	/*
	//Direction: 0 - Left, 1 - Up, 2 - Right, 3 - Down
	switch (activeNode -> parent -> layout) {

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
		if (!strcmp(tokens[1], "vertical")) {
			activeNode -> parent ->  layout = 0; 
		} else if (!strcmp(tokens[1], "horizontal")) { 
			activeNode -> parent -> layout = 1; 
		} else if (!strcmp(tokens[1], "grid")) {
			activeNode -> parent -> layout = 2;
		}

		placeNode(activeNode -> parent,
				(activeNode -> parent) -> x,     (activeNode -> parent) -> y,
				(activeNode -> parent) -> width, (activeNode -> parent) -> height);

	} else if (!strcmp(tokens[0], "focus")) {
		if (!strcmp(tokens[1], "left"))
			focus(0);
		else if (!strcmp(tokens[1], "up"))
			focus(1);
		else if (!strcmp(tokens[1], "right"))
			focus(2);
		else if (!strcmp(tokens[1], "down"))
			focus(3);

	} else if (!strcmp(tokens[0], "containerize")) {
		if (!strcmp(tokens[1], "client")) {
			if (activeNode -> previous != NULL) {
				fprintf(stderr,"Containerizing");
				Node * newContainer = allocateNode();
				Node * nodeParent = activeNode -> parent;

				parentNode(activeNode, newContainer);
				parentNode(newContainer, nodeParent);
				activeNode = newContainer -> child;
				
				placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
				focusNode(newContainer -> child);

			}	else {
				fprintf(stderr, "Containerize called but alone in contianer");
			}
		}

	} else if (!strcmp(tokens[0], "view")) {

		if (!strcmp(tokens[1], "parent")) {
			if (viewNode -> parent != NULL) {
				unmapNode(viewNode);
				viewNode = viewNode -> parent;
				placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
			}

		} else if (!strcmp(tokens[1], "child")) {

			if (activeNode != viewNode) {
				Node *n = activeNode;
				while (n -> parent != viewNode && n != NULL) { n = n -> parent; }

				unmapNode(viewNode);
				viewNode = n;
				placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
			}
		}
	} else if (!strcmp(tokens[0], "kill")) {
		if (!strcmp(tokens[1], "client")) {
			dumpTree();
			fprintf(stderr, "Destroy Client %p\n", activeNode);

			if (isClient(activeNode)) {
				destroyNode(activeNode);


				dumpTree();

				placeNode(viewNode, 
						viewNode -> x, viewNode -> y, 
						viewNode -> width, viewNode -> height);
			}
		} else if (!strcmp(tokens[1], "container")) {
			dumpTree();
			fprintf(stderr, "Destroy Container %p\n", activeNode -> parent);
			//destroyContainer(currentContainer);
			dumpTree();
		}
	}
}
