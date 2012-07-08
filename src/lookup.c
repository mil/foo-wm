#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>


#include "fifo-wm.h"
#include "util.h"

//Returns the client associated with given windowv
Node * getNodeByWindow(Window * window) {
	Lookup *entry;
	for (entry = lookup; entry != NULL; entry = entry -> previous)
		if (((int) *window) == entry -> window) 
			return entry -> node;

	return NULL;
}

void removeLookupEntry(Window * window) {
	Lookup *cut;
	Lookup *entry;

	//Removing last entry in list 
	if (lookup -> window == (int) *window) {
		entry = lookup -> previous;
		free(lookup);
		lookup = entry;
		return;
	}

	//Rmove any other entry in list
	for	(entry = lookup; entry -> previous != NULL; entry = entry -> previous) {
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
	entry -> previous = lookup;
	lookup = entry;
}
