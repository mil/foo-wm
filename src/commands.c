#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fifo-wm.h"
#include "commands.h"
#include "tree.h"
#include "util.h"

/* Extracts the next token from the tokenString */
char * nextToken(char ** tokenString) {
	char *command;
	command = strsep(tokenString, " ");
	if (!command) return NULL;

	char *newLine = strchr(command, '\n');
	if (newLine) *newLine = '\0';

	return command;
}

void handleCommand(char* request) {
	fprintf(stderr, "Recv from FIFO: %s", request);

	char *tokens[5]; char *token; int i = 0;
	while ((token = nextToken(&request))) {
		tokens[i] = token;
		i++;
	}

	if (!strcmp(tokens[0], "dump"))
		dumpTree();
	else if (!strcmp(tokens[0], "layout"))
		layout(tokens[1]);
	else if (!strcmp(tokens[0], "focus"))
		focus(tokens[1], atoi(tokens[2]));
	else if (!strcmp(tokens[0], "move"))
		move(atoi(tokens[1]));
	else if (!strcmp(tokens[0], "containerize"))
		containerize();
	else if (!strcmp(tokens[0], "zoom"))
		zoom(atoi(tokens[1]));
	else if (!strcmp(tokens[0], "kill"))
		kill();
}

void layout(char * l) {
	fprintf(stderr, "Setting layout to: %s", l);

	Node *setNode = isClient(focusedNode) ?  focusedNode -> parent : focusedNode;
	int newLayout = 0;

	if      (!strcmp(l, "vertical"))   newLayout = VERTICAL;
	else if (!strcmp(l, "horizontal")) newLayout = HORIZONTAL;
	else if (!strcmp(l, "grid"))       newLayout = GRID;
	else if (!strcmp(l, "max"))        newLayout = MAX;

	setNode -> layout = newLayout;
	placeNode(setNode, 
			setNode -> x, setNode -> y, setNode -> width, setNode -> height);
}

//Moves the current selection given amount
void move(int amount) {
	Node *startNode = focusedNode;
	Node *swapNode = getBrother(startNode, amount);

	swapNodes(startNode, swapNode);
	focusNode(focusedNode, NULL, True);
}


/* Updates the viewNode approximating the current focusNode */
void zoom(int level) {
	while (level < 0) {
		if (viewNode -> parent) { 
			unmapNode(viewNode);
			viewNode = viewNode -> parent;
		}	else { return; }
		placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
		focusNode(focusedNode, NULL, True);
		level++;
	}
	while (level > 0) {
		if (focusedNode == viewNode) return;
		Node *n = focusedNode;
		while (n && n -> parent != viewNode) n = n -> parent;
		if (!n) return;

		unmapNode(viewNode);
		viewNode = n;
		//TODO:: Need placeNode to be aware of focusnnode (in)active colors....
		placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
		level--;
	}
}

void focus(char * brotherOrPc, int delta) {
	fprintf(stderr, "Cycling focus");
	int brotherSwitch = -1;
	if (!strcmp(brotherOrPc, "brother")) brotherSwitch = 1;
	else if (!strcmp(brotherOrPc, "pc")) brotherSwitch = 0;
	else return;

	while (delta != 0) {
		Node * newFocus;

		if (brotherSwitch) {
			newFocus = getBrother(focusedNode, (delta < 0) ? -1 : 1);
		} else {
			newFocus = delta < 0 ? 
				focusedNode -> parent : 
				(focusedNode -> focus ? focusedNode -> focus : focusedNode -> child);
		}

		focusNode(newFocus, NULL, True);
		delta = delta + ( delta > 0 ? -1 : 1);	
	}
}

void containerize() {
	if (!focusedNode) return;
	if (!(focusedNode -> previous || focusedNode -> next)) return;
	if (focusedNode -> child && !isClient(focusedNode -> child))
		if (!(focusedNode -> child -> previous || focusedNode -> child -> previous))
			return;

	Node * newContainer    = allocateNode();
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
		focusNode(newFocus, NULL, True);
		placeNode(viewNode, 
				viewNode -> x, viewNode -> y, 
				viewNode -> width, viewNode -> height);
	}
}
