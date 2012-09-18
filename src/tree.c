#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "foo-wm.h"
#include "tree.h"
#include "lookup.h"
#include "util.h"
#include "window.h"

/* --------------------------------------------------------------------------
 * Bool Returns 
 * -------------------------------------------------------------------------- */
Bool isClient(Node * node) { 
  /* Is the node a client? */
  if (node && (node -> window != (Window) NULL)) return True;
  else return False;  
}

Bool isOnlyChild(Node * node) { 
  /* Is the node an only child */
  if (node && (node -> next || node -> previous)) return False;
  else return True;
}

Bool nodeIsParentOf(Node * nodeA, Node * nodeB) {
  /* Searches nodeA for an occurance of nodeB
   * if successful, return true */
  if (nodeA == nodeB) return True;

  Node *n = NULL;
  for (n = nodeA -> child; n; n = n -> next) {
    if (nodeIsParentOf(n, nodeB))
      return True;
  }

  return False;
}

Bool unfocusNode(Node * n, Bool focusPath) {
  /* Unfocuses the currently focused node, called only by focusNode 
   * Returns Bool if an update of the view is needed
   * Dangerous if called alone */
  if (!n) return False;

  Bool setView = (n == viewNode) ? True : False;
  fprintf(stderr, "Yo i be unfocusing %p\n", n);

  //Unfocusing Code for previous focusedNode
  if (isClient(n)) {

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

/* --------------------------------------------------------------------------
 * Char * Returns 
 * -------------------------------------------------------------------------- */
char * crawlNode(Node * node, int level) {
  char * returnString = "";
  int j; for (j = level; j > 0; j--) { returnString = stringAppend(returnString, "|\t"); }

  char nodeInfo[1000];
  if (isClient(node)) {

    sprintf(nodeInfo, "Client (%p)", node);
    returnString = stringAppend(returnString, nodeInfo);

    if (node == rootNode) { returnString = stringAppend(returnString, " [Root]"); }
    if (node == focusedNode) { returnString = stringAppend(returnString, " [Focused]"); }
    if (node == viewNode) { returnString = stringAppend(returnString, " [View]"); }
    returnString = stringAppend(returnString, "\n");

  } else {
    char *label = NULL;
    switch (node -> layout) {
      case VERTICAL   : label = "Vertical"; break;
      case HORIZONTAL : label = "Horizontal"; break;
      case GRID       : label = "Grid"; break;
      case MAX        : label = "Max"; break;
      case FLOAT      : label = "Float"; break;

    }

    sprintf(nodeInfo, "Container (%p) %s (focus=%p)", node, label, node -> focus);
    returnString = stringAppend(returnString, nodeInfo);

    if (node == focusedNode) returnString = stringAppend(returnString, "[Focused]");
    if (node == viewNode)    returnString = stringAppend(returnString, " [View]");
    returnString = stringAppend(returnString, "\n");

    Node *n = NULL;
    for (n = node -> child; n; n = n -> next)
      returnString = stringAppend(returnString, crawlNode(n, level + 1));
  }

  return returnString;
}



/* --------------------------------------------------------------------------
 * Long Returns 
 * -------------------------------------------------------------------------- */
long getBorderColor(Node * node, Bool focusPath) {
  if (focusPath) {
    if (focusedNode == node)
      return activeFocusedColor;
    else
      return node -> parent -> focus == node ? 
        inactiveFocusedColor : activeUnfocusedColor;

  } else {
    return node -> parent -> focus == node ? 
      inactiveFocusedColor: inactiveUnfocusedColor;
  }
}


/* --------------------------------------------------------------------------
 * Node Returns 
 * -------------------------------------------------------------------------- */
Node * getBrother(Node * node, int delta) {
  if (!node) return NULL;

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



/* --------------------------------------------------------------------------
 * Void Returns 
 * -------------------------------------------------------------------------- */
void brotherNode(Node *node, Node * brother, int position) {
  if (!node || !brother) return;
  node -> parent = brother -> parent;

  if (position == 0) {
    node -> next = brother;
    if (!brother -> previous) { //Pop in the front
      node -> parent -> child = node;
    } else {
      //Shift previous pointer
      node -> previous = brother -> previous;
      if (node -> previous) node -> previous -> next = node;
    }
    brother -> previous = node;
  } else if (position == 1) {
    node -> previous = brother;
    node -> next = brother -> next;
    if (node -> next) node -> next -> previous = node;
    brother -> next = node;
  }
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
  fprintf(stderr, "Made it here");

  if (n == focusedNode) focusedNode = NULL;
  fprintf(stderr, "n is %p", n);

  //Recursivly unmap down all children of the node
  if (isClient(n)) {
    removeLookupEntry(&n -> window);
    XDestroyWindow(display, n -> window);
    free(n);
  } else if (n) {
    Node *destroy = n -> child; Node *next = NULL;
    do {
      if (destroy) {
        next = destroy -> next;
        destroyNode(destroy);
      } else { next = NULL; }
    } while (next);

    //if (n -> parent && n -> parent -> focus == n) n -> parent -> child = NULL;
    //free(n);
  }

}


void dumpTree() {
  fprintf(stderr, "Printing the tree\n");
  fprintf(stderr, "----------------------------------\n");
  crawlNode(rootNode, 0);
  fprintf(stderr, "----------------------------------\n");
}


//This should focus OR select
void focusNode(Node * n, XEvent * event, Bool setFocused, Bool focusPath) {
  if (!n || n == focusedNode) return;
  fprintf(stderr, "Focusing %p", n);

  Node *oldFocus = focusedNode;


  if (focusPath && setFocused) { 
    fprintf(stderr, "\n\nNode %p, is in the focus patho\n\n", n);
    unfocusNode(focusedNode, True);
    if (setFocused && n -> parent)   n -> parent -> focus = n;
  }

  if (setFocused)  {
    focusedNode = n;

    if (oldFocus && nodeIsParentOf(viewNode, oldFocus))
      rePlaceNode(oldFocus);
  
    if (oldFocus == viewNode) viewNode = n;
  }

  //if (setView)     viewNode    = n;

  // Are we at the bottom level 
  if (isClient(n)) {
    if (n -> parent)  {
      rePlaceNode(n -> parent);
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
  if (viewNode == n)
    placeNode(viewNode, rootX, rootY, rootWidth, rootHeight);
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

void placeNode(Node * node, int x, int y, int width, int height) {
  if (!node) return;
  node -> x = x; node -> y = y; node -> width = width; node -> height = height;
  fprintf(stderr, "Place Node XY:[%d, %d], WH:[%d, %d]\n", x, y, width, height);

  if (isClient(node)) {
    fprintf(stderr,"Rendering window\n");
    XMapWindow(display, node -> window);
    XRaiseWindow(display, node -> window);
    XMoveResizeWindow(display, node -> window, 
        (x < 0) ? 0 : x, (y < 0) ? 0 : y, 
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

    XSetWindowBorder(display, node -> window, getBorderColor(node, inFocusPath));

  } else {
    //Count up children prior to loop
    int children = 0; int i = 0; Node *a = NULL;
    if (!node -> child) return;
    for (a = node -> child; a; a = a -> next) children++;

    /* Determine the number of rows and cols */
    int rows; int cols;
    switch (node -> layout) {
      /* Emulate a grid  regardless */
      case VERTICAL  : cols = children; rows = 1; break;
      case HORIZONTAL: cols = 1; rows = children; break;
      case GRID      : gridDimensions(children, &rows, &cols); break;
      case MAX       : cols = 1; rows = 1; break;
    }

    Bool callPlace;
    int pad;
    for (a = node -> child; a; a = a -> next, i++) {

      if (node -> layout == FLOAT) {
        placeNode(a, a -> x + 10, a -> y + 10 , a -> width - 20, a -> height - 20);

      } else { /* Rendering based on a grid style */

        pad = isClient(a) ? clientPadding : containerPadding;
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

          if (node -> layout == GRID) { /* Must account for prime case */
            if (children == 2)      a -> height = height - (pad * 2);
            if (i + 1 == children)  a -> width = x + width - a -> x - (pad * 2);
          }
          rePlaceNode(a);
        } else {
          fprintf(stderr, "Going to call unmap on %p\n", a);
          unmapNode(a);
        }

      }
    }
  }
}

void rePlaceNode(Node * node) {
  placeNode(node, node -> x, node -> y, node -> width, node -> height);
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


void unmapNode(Node * node) {
  if (isClient(node)) {
    XUnmapWindow(display, node -> window);
  } else {
    Node *n = NULL;
    for (n = node -> child; n; n = n -> next)
      unmapNode(n);
  }
}


void unparentNode(Node *node) {
  if (!(node && node -> parent)) return;

  fprintf(stderr, "Unparent called\n");
  //Move parent's child pointer if were it....
  if (node -> parent -> child == node)
    node -> parent -> child = node -> next;
  if (node -> parent -> focus == node)
    node -> parent -> focus = node -> parent -> child;

  //Move the next and previous pointers to cut out the node
  if (node -> next)     node -> next -> previous = node -> previous;
  if (node -> previous) node -> previous -> next = node -> next;

  //Set our parent to NULL
  Node * oldParent = node -> parent;
  node -> parent = NULL; node -> next = NULL; node -> previous = NULL;
  if (!oldParent -> child)
    destroyNode(oldParent);
}
