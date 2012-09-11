#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "foo-wm.h"
#include "util.h"


//Thank you DWM ;)
unsigned long getColor(const char *colstr) {
  Colormap cmap = DefaultColormap(display, activeScreen);
  XColor color;

  if(!XAllocNamedColor(display, cmap, colstr, &color, &color)) { return 0; }
  return color.pixel;
}


int xError(XErrorEvent *e) {
  fprintf(stderr, "XErrorEvent of Request Code: %d and Error Code of %d\n", e -> request_code, e -> error_code);
  return 0;
}

Node * allocateNode() {
  Node *n = malloc(sizeof(Node));
  n -> previous = NULL; n -> next = NULL;
  n -> parent = NULL;   n -> child = NULL;
  n -> focus = NULL;    
  n -> window = (Window) NULL; 
  n -> layout = defaultLayout;
  return n;
}

void recalculateRootDimensions (void) {
  if (!rootNode) return;
  rootNode -> x         = rootX      = screenPaddingLeft;
  rootNode -> y         = rootY     = screenPaddingTop;
  rootNode -> width     = rootWidth = DisplayWidth(display, activeScreen) - screenPaddingLeft - screenPaddingRight;
  rootNode -> height    = rootHeight = DisplayHeight(display, activeScreen) - screenPaddingTop - screenPaddingBottom;
}

void gridDimensions(int children, int * rows, int * cols) {
  if (children == 2) { *rows = 1; *cols = 2; return; } /* Edgecase */

  int square = (int) sqrt(children);
  int r = square;

  while (((children % r) != 0)) { r++; }
  int c = children / r;

  if ((r == 1 && c != 1) || (c == 1 && r != 1)) {
    gridDimensions(children + 1, rows, cols);
  } else {
    *rows = r;
    *cols = c;
  }
}

Bool isPrime(int number) {
  Bool prime = True;
  int i;
  for(i = 2; i <= number / 2; i++)
    if(number % i == 0) { prime = False; break; } 
  return prime;
}

Node * focusOrChildOf(Node * node) {
  if (!node) return NULL;
  else if (node -> focus) return node -> focus;
  else return node -> child;
}

int directionStringToInt(char * directionString) {
  if (!strcmp(directionString, "left"))       return LEFT; 
  else if (!strcmp(directionString, "up"))    return UP;
  else if (!strcmp(directionString, "right")) return RIGHT; 
  else if (!strcmp(directionString, "down"))  return DOWN;
  else return -1;
}

int bytesUntilNull(char * string) {
  int counter = 0;
  while (string[counter] != '\0')
    counter++;

  /* Include the NULL Byte */
  return ++counter;
}

char * stringAppend(char ** originalString, char * appendContent) {
  char * returnString = (char *) malloc(strlen(originalString) + strlen(appendContent));
  strcpy(returnString, originalString); 
  strcat(returnString, appendContent);

  return returnString;
}
