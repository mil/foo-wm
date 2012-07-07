#include <X11/Xlib.h>
#include <stdlib.h>

#include "fifo-wm.h"
#include "util.h"

//Returns the client associated with given windowv
Node * getNodeByWindow(Window * window) {
	Lookup *entry;
	for (entry = lookup; entry != NULL; entry = entry -> previous) {
		if (((int) *window) == entry -> window)
			return entry -> node;
	}

	return NULL;
}

Node * removeLookupEntry(Window * window) {
	Lookup *entry;
	for	(entry = lookup; entry != NULL; entry = entry -> previous) {
	}
	return NULL;
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
