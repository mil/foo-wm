#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>


#include "foo-wm.h"
#include "util.h"

//Returns the client associated with given windowv
Node * getNodeByWindow(Window * window) {
  Lookup *entry;
  for (entry = lookupTail; entry != NULL; entry = entry -> previous)
    if (((int) *window) == entry -> window) 
      return entry -> node;

  return NULL;
}

Node * getNodeById(int id) {
  Lookup *entry;
  for (entry = lookupTail; entry != NULL; entry = entry -> previous)
    if (id == (int) entry -> node -> window) 
      return entry -> node;

  return NULL;
}

void removeLookupEntry(Window * window) {
  Lookup *cut = NULL;
  Lookup *entry = NULL;

  //Removing last entry in list 
  if (lookupTail -> window == (int) *window) {
    entry = lookupTail -> previous;
    free(lookupTail);
    lookupTail = entry;
    return;
  }

  //Rmove any other entry in list
  for (entry = lookupTail; entry -> previous != NULL; entry = entry -> previous) {
    if ((int)*window == entry -> previous -> window) {
      cut = entry -> previous;
      entry -> previous = entry -> previous -> previous;
      free(cut);
      return;
    }
  }
}

void addLookupEntry( Node * node, Window * window) {
  //Add Client and window to lookup list
  Lookup *entry = malloc(sizeof(Lookup));
  entry -> node = node;
  entry -> window = (int) *window;

  //Set lookup (last item) as aour previous, and now were lookup (last)
  entry -> previous = lookupTail;
  lookupTail = entry;
}
