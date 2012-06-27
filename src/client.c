#include <X11/Xlib.h>

#include "fifo-wm.h"
#include "client.h"


void focusClient(Client *client) {
	XSetWindowBorder(display, currentClient -> window, unfocusedColor);
	XSetWindowBorderWidth(display, currentClient -> window, 0);

	XSetWindowBorderWidth(display, client -> window, 2);
	XSetWindowBorder(display, client -> window, focusedColor);

	//Focuses window
	XSelectInput(
			display, client -> window, 
			FocusChangeMask | KeyPressMask | 
			ButtonPressMask | LeaveWindowMask | OwnerGrabButtonMask
			);
	XGrabButton(
			display, AnyButton, AnyModifier, client -> window, False,
			OwnerGrabButtonMask | ButtonPressMask,
			GrabModeSync, GrabModeSync, None, None);
}
