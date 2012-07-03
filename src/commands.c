#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fifo-wm.h"
#include "commands.h"
#include "tree.h"
#include "util.h"


void focus(int direction) {
	//Direction: 0 - Previous, 1 - Next, 2 - Parent
	if (direction == 0) {
		if (activeNode -> next != NULL) {
			focusNode(activeNode -> next);
		} else {
			focusNode(activeNode -> parent -> child);
		}
	} else if (direction == 1) {
		if (activeNode -> previous != NULL) {
			focusNode(activeNode -> previous);
		} else {
			Node *last = activeNode -> parent -> child;
			while (last -> next != NULL) { last = last -> next; }
			focusNode(last);

		}
	} else if (direction == 2) {
		/*
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
		*/
	}
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
		if (!strcmp(tokens[1], "vertical")) {	(activeNode -> parent) ->  layout = 0; 
		} else if (!strcmp(tokens[1], "horizontal")) { (activeNode -> parent) -> layout = 1; }
		placeNode(activeNode -> parent,
				(activeNode -> parent) -> x,     (activeNode -> parent) -> y,
				(activeNode -> parent) -> width, (activeNode -> parent) -> height);

	} else if (!strcmp(tokens[0], "focus")) {
		if (!strcmp(tokens[1], "next"))
			focus(0);
		else if (!strcmp(tokens[1], "previous"))
			focus(1);
		else if (!strcmp(tokens[1], "parent"))
			focus(2);

	} else if (!strcmp(tokens[0], "containerize")) {
		if (activeNode -> previous != NULL) {
			fprintf(stderr,"Containerizing");
			Node * newContainer = malloc(sizeof(Node));	
			newContainer -> next = NULL; newContainer -> previous = NULL;
			newContainer -> parent = NULL; newContainer -> child = NULL;
			Node * nodeParent = activeNode -> parent;

			parentNode(activeNode, newContainer);
			parentNode(newContainer, nodeParent);
			activeNode = newContainer -> child;

			placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
		}	else {
			fprintf(stderr, "Containerize called but alone in contianer");
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

			destroyNode(activeNode);

			dumpTree();

			placeNode(viewNode, 
					viewNode -> x, viewNode -> y, 
					viewNode -> width, viewNode -> height);
		} else if (!strcmp(tokens[1], "container")) {
			dumpTree();
			fprintf(stderr, "Destroy Container %p\n", activeNode -> parent);
			//destroyContainer(currentContainer);
			dumpTree();
		}
	}
}
