#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foo-wm.h"
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

char * handleCommand(char * request) {
  fprintf(stderr, "Recv from Socket: %s", request);

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
    focus(tokens[1], tokens[2]);
  else if (!strcmp(tokens[0], "move"))
    move(atoi(tokens[1]));
  else if (!strcmp(tokens[0], "shift"))
    shift(tokens[1]);
  else if (!strcmp(tokens[0], "containerize"))
    containerize();
  else if (!strcmp(tokens[0], "zoom"))
    zoom(atoi(tokens[1]));
  else if (!strcmp(tokens[0], "kill"))
    kill();

  XFlush(display);
  char * response = "I'm sending back to the socket";
  return response;
}

void layout(char * l) {
  fprintf(stderr, "Setting layout to: %s", l);

  Node *setNode = isClient(focusedNode) ?  focusedNode -> parent : focusedNode;
  int newLayout = 0;

  if      (!strcmp(l, "vertical"))   newLayout = VERTICAL;
  else if (!strcmp(l, "horizontal")) newLayout = HORIZONTAL;
  else if (!strcmp(l, "grid"))       newLayout = GRID;
  else if (!strcmp(l, "max"))        newLayout = MAX;
  else if (!strcmp(l, "float"))      newLayout = FLOAT;


  setNode -> layout = newLayout;
  placeNode(setNode, 
      setNode -> x, setNode -> y, setNode -> width, setNode -> height);
}

//Moves the current selection given amount
void move(int amount) {
  Node *swapNode = getBrother(focusedNode, amount);

  swapNodes(focusedNode, swapNode);
  focusNode(focusedNode, NULL, True, True);
}


void shift(char * directionString) {
  int direction = directionStringToInt(directionString);
  Node * insertNode = getBrotherByDirection(focusedNode, direction);
  if (!insertNode) { insertNode = getClientByDirection(focusedNode, direction); }

  Node * oldParent = focusedNode -> parent;
  unparentNode(focusedNode);
  placeNode(oldParent, oldParent -> x, oldParent -> y, oldParent -> width, oldParent -> height);

  /* Shifting within our current container */
  if (insertNode) {
    while (!isClient(insertNode)) {
      insertNode = insertNode -> focus ? insertNode -> focus : insertNode -> child;
    }
    brotherNode(focusedNode, insertNode, direction == LEFT || direction == UP ? 0 : 1); 
    placeNode(focusedNode -> parent, focusedNode -> parent -> x, focusedNode -> parent -> y,
        focusedNode -> parent -> width, focusedNode -> parent -> height);

    focusNode(focusedNode, NULL, True, True);
  } 
}


/* Updates the viewNode approximating the current focusNode */
void zoom(int level) {
  while (level < 0) {
    if (viewNode -> parent) { 
      unmapNode(viewNode);
      viewNode = viewNode -> parent;
    } else { return; }
    placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
    focusNode(focusedNode, NULL, True, True);
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
    
    fprintf(stderr, "placeing yo");
    placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
    if (focusedNode == viewNode && !isClient(focusedNode)) {
      focusNode(focusOrChildOf(focusedNode), NULL, True, True);
    }
    level--;
  }
}

void focus(char * argA, char * argB) {
  if (!strcmp(argA, "direction")) {
    int direction = directionStringToInt(argB);
    Node *newFocus = getClientByDirection(focusedNode, direction);
    fprintf(stderr, "Directional focusing %p\n", newFocus);
    focusNode(newFocus, NULL, True, True);

  } else {  
    int delta = atoi(argB);

    fprintf(stderr, "Cycling focus");
    int brotherSwitch = -1;
    if (!strcmp(argA, "brother")) brotherSwitch = 1;
    else if (!strcmp(argA, "pc")) brotherSwitch = 0;
    else return;

    while (delta != 0) {
      Node * newFocus;

      if (brotherSwitch) {
        newFocus = getBrother(focusedNode, (delta < 0) ? -1 : 1);
      } else {
        newFocus = (delta < 0) ? 
          focusedNode -> parent : focusOrChildOf(focusedNode);
      }

      fprintf(stderr, "Going to focus node: %p", newFocus);

      focusNode(newFocus, NULL, True, True);
      delta = delta + ( delta > 0 ? -1 : 1);  
    }
  }
}

void containerize(void) {
  if (!focusedNode) return;
  if (focusedNode -> child && !isClient(focusedNode -> child))
    if (isOnlyChild(focusedNode -> child)) return;

  Node *insertNode, *newContainer = allocateNode(); int insertPosition;
  if (focusedNode -> parent && focusedNode -> parent -> focus == focusedNode)
    focusedNode -> parent -> focus = newContainer;

  if (focusedNode -> previous) {
    insertNode = focusedNode -> previous; insertPosition = NEXT;
  } else {
    insertNode = focusedNode -> next; insertPosition = PREVIOUS;
  }

  parentNode(focusedNode, newContainer);
  brotherNode(newContainer, insertNode, insertPosition);
  placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
}

void kill(void) {
  dumpTree();
  fprintf(stderr, "Destroying Client %p\n", focusedNode);

  if (isClient(focusedNode)) {

    /* Save closest client and destroy node */
    Node *newFocus = getClosestClient(focusedNode);

    if (focusedNode == viewNode) viewNode = viewNode -> parent;

    if ( isOnlyChild(focusedNode) && focusedNode -> parent) {
      viewNode = focusedNode -> parent -> parent ?
        focusedNode -> parent -> parent : focusedNode -> parent;
    }

    destroyNode(focusedNode);
    dumpTree();

    /* Give the closeset client of destroyed node focus and rerender */
    focusNode(newFocus, NULL, True, True);
    placeNode(viewNode, 
        viewNode -> x, viewNode -> y, viewNode -> width, viewNode -> height);
  }
}

