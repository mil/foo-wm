#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foo-wm.h"
#include "commands.h"
#include "lookup.h"
#include "responses.h"
#include "tree.h"
#include "util.h"

/* -----------------------------------------------------------------------------
 * Handeling Commands 
 * ---------------------------------------------------------------------------*/
char * handleCommand(char * request) {
  fprintf(stderr, "Recv from Socket: %s", request);

  char *tokens[5]; char *token; int i = 0;
  while ((token = nextToken(&request))) {
    tokens[i] = token;
    i++;
  }

  char * response = "Some arbitrary response back to the socket\0";

  if (!strcmp(tokens[0], "dimensions"))
    dimensions(tokens[1], tokens[2]);


  if (!strcmp(tokens[0], "absorb"))
    absorb(tokens[1], tokens[2]);
  if (!strcmp(tokens[0], "containerize"))
    containerize();
  else if (!strcmp(tokens[0], "focus"))
    focus(tokens[1], tokens[2]);
  else if (!strcmp(tokens[0], "get"))
    response = get(tokens[1]);
  else if(!strcmp(tokens[0], "jump"))
    jump(tokens[1]);
  else if (!strcmp(tokens[0], "kill"))
    kill();
  else if (!strcmp(tokens[0], "layout"))
    layout(tokens[1]);
  else if (!strcmp(tokens[0], "mark"))
    mark(tokens[1]);
  else if (!strcmp(tokens[0], "set"))
    set(tokens[1], tokens[2]);
  else if (!strcmp(tokens[0], "shift"))
    shift(tokens[1], atoi(tokens[2]));
  else if (!strcmp(tokens[0], "swap"))
    swap(tokens[1], tokens[2]);
  else if (!strcmp(tokens[0], "zoom"))
    zoom(atoi(tokens[1]));

  XFlush(display);
  return response;
}


/* Extracts the next token from the tokenString */
char * nextToken(char ** tokenString) {
  char *command;
  command = strsep(tokenString, " ");
  if (!command) return NULL;

  char *newLine = strchr(command, '\n');
  if (newLine) *newLine = '\0';

  return command;
}


/* -----------------------------------------------------------------------------
 * IPC Commands 
 * ---------------------------------------------------------------------------*/
void dimensions(char * w, char * h) {
  int width  = atoi(w);
  int height = atoi(h);
  rootNode -> x = rootX = screenPaddingLeft;
  rootNode -> y = rootY = screenPaddingTop;
  rootNode -> width = rootWidth = width - screenPaddingLeft - screenPaddingRight;
  rootNode -> height = rootHeight = height - screenPaddingTop - screenPaddingBottom;
}


void absorb(char * argA, char * argB) {
  /* Absorbs the given node into the container of the focused node 
   * If the focused node is a client, containerize will be called then absorb
   * */
  if (!focusedNode) return;

  if (isClient(focusedNode)) containerize();

}

void containerize(void) {
  if (!focusedNode) return;
  if (focusedNode -> parent && focusedNode -> parent -> child && !isClient(focusedNode -> parent))
    if (isOnlyChild(focusedNode)) return;

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


void focus(char * argA, char * argB) {
  if (!focusedNode) return;

  int delta = atoi(argB);
  Node * newFocus = NULL;

  if (!strcmp(argA, "id")) {
    newFocus = getNodeById(delta);

  } else if (!strcmp(argA, "brother")) {
      newFocus = getBrother(focusedNode, delta);
  } else if (!strcmp(argA, "pc")) {
    while (delta != 0) {
      newFocus = (delta < 0) ? 
          focusedNode -> parent : focusOrChildOf(focusedNode);
      delta = delta + ( delta > 0 ? -1 : 1);  
      focusNode(newFocus, NULL, True, True);
    } return;
  }

  focusNode(newFocus, NULL, True, True);
}

char * get(char * property) {
  if (!strcmp(property, "tree"))
    return jsonTree(rootNode, 0);
  else if (!strcmp(property, "view"))
    return jsonTree(viewNode, 0);
  else if (!strcmp(property, "focus"))
    return jsonTree(focusedNode, 0);
  else if (!strcmp(property, "marks"))
    return jsonMarks();

  return "";
}

void kill(void) {
  fprintf(stderr, "Destroying Client %p\n", focusedNode);

  if (isClient(focusedNode)) {

    /* Save closest client and destroy node */
    Node *newFocus = getClosestClient(focusedNode);

    if (focusedNode == viewNode) viewNode = viewNode -> parent;

    if ( isOnlyChild(focusedNode) && focusedNode -> parent) {
      viewNode = focusedNode -> parent -> parent ?
        focusedNode -> parent -> parent : focusedNode -> parent;
    }

    Node *oldFocus = focusedNode;
    focusNode(newFocus, NULL, True, True);
    destroyNode(oldFocus);

    rePlaceNode(viewNode);
  }
}

void jump(char * markName) {
  fprintf(stderr, "Finding a node");

  Mark *n = NULL;
  for(n = markTail; n; n = n -> previous) {
    if (!strcmp(n -> name, markName)) {
      fprintf(stderr, "Going to focus mark %p", n -> node);
      unmapNode(viewNode);
      viewNode = n -> node;
      placeNode(n -> node, rootX, rootY, rootWidth, rootHeight);
      focusNode(n -> node, NULL, True, True);
      while (!isClient(focusedNode)) focus("pc", "1");
    }
  }
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
  rePlaceNode(setNode);
}


/* Adds the current viewNode as a mark */
void mark(char * markName) {

  Mark *m = malloc(sizeof(Mark));
  m -> name = malloc(sizeof(markName));
  strcpy(m -> name, markName);
  m -> node     = viewNode;
  m -> previous = markTail;
  markTail = m;

  fprintf(stderr, "\nAdded the mark::  %s // %s\n", markName, markTail -> name);
}

void set(char * property, char * value) {

  if (!strcmp(property, "client_border_width")) {
    border = atoi(value);
  } else if (!strcmp(property, "container_padding")) {
    containerPadding = atoi(value);
  } else if (!strcmp(property, "client_padding")) {
    clientPadding = atoi(value);
  } else if (!strcmp(property, "screen_padding_top")) {
    screenPaddingTop = atoi(value);
  } else if (!strcmp(property, "screen_padding_left")) {
    screenPaddingLeft = atoi(value);
  } else if (!strcmp(property, "screen_padding_right")) {
    screenPaddingRight = atoi(value);
  } else if (!strcmp(property, "screen_padding_bottom")) {
    screenPaddingBottom = atoi(value);
  }

  recalculateRootDimensions();
  rePlaceNode(viewNode);

}

//Moves the current selection given amount
void shift(char * argA, int delta) {

  int brotherSwitch = -1;
  if (!strcmp(argA, "brother")) brotherSwitch = 1;
  else if (!strcmp(argA, "pc")) brotherSwitch = 0;
  else return;

  while (delta != 0) {
    if (brotherSwitch) {
      Node *swapNode = getBrother(focusedNode, delta);
      swapNodes(focusedNode, swapNode);
      focusNode(focusedNode, NULL, True, True);
      delta = 0;
    } else {
      if (delta > 0) { return; } else {
        if (focusedNode -> parent && focusedNode -> parent -> parent) {
          Node *newParent = focusedNode -> parent -> parent;
          parentNode(focusedNode, newParent);
          rePlaceNode(newParent);
        } else { return; }
        delta++;
      }
    }
  }

}

/* Swaps two nodes in place based on node ids */
Bool swap(char * argA , char * argB) {
  int idA     = atoi(argA),
      idB     = atoi(argB);
  Node *nodeA = getNodeById(idA),
       *nodeB = getNodeById(idB);

  if (!nodeA || !nodeB) {
    return False;
  } else {
    swapNodes(nodeA, nodeB);
    return True;
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
