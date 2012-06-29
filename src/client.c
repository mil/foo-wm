#include <X11/Xlib.h>
#include <stdio.h>

#include "fifo-wm.h"
#include "client.h"


void focusClient(Client *client) {

	fprintf(stderr, "Focusing client window %p\n", &client -> window);

	Client *c;
	for (c = (client -> parent) -> client; c != NULL; c = c -> next) {
		if (client == c) {
			XSetWindowBorder(display, client -> window, focusedColor);
		} else {
			XSetWindowBorder(display, c -> window, unfocusedColor);
		}
	}

	(client -> parent) -> focus = client;

	//Focuses window
	XSelectInput(
			display, client -> window, 
			FocusChangeMask | KeyPressMask | 
			ButtonPressMask | LeaveWindowMask | OwnerGrabButtonMask
			);
	XGrabButton(
			display, AnyButton, AnyModifier, client -> window, False,
			OwnerGrabButtonMask | ButtonPressMask,
			GrabModeSync, GrabModeSync, None, None
			);

	XRaiseWindow(display, client -> window);
	XSetInputFocus(display, client -> window, RevertToPointerRoot, CurrentTime);
}
