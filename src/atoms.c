#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "atoms.h"
#include "foo-wm.h"

void sendDeleteWindow(Window * window) {
  int protocolsReturn = -1;
  Atom *protocols     = NULL;
  XGetWMProtocols(display, *window, &protocols, &protocolsReturn);

  XEvent dw;
  dw.type                 = ClientMessage;
  dw.xclient.window       = *window;
  dw.xclient.format       = 32;
  dw.xclient.message_type = icccmAtoms[WmProtocols];
  dw.xclient.data.l[0]    = icccmAtoms[WmDeleteWindow];
  dw.xclient.data.l[1]    = CurrentTime;
  XSendEvent(display, *window, False, NoEventMask, &dw);
}

void setNetActive(Window * window) {
  XChangeProperty(display, root, ewmhAtoms[NetActive], 
      XA_WINDOW, 32, PropModeReplace, (unsigned char *) window, 1);
}


void setupAtoms(void) {
  // Set Up ICCCM Atoms for:
  // WM_PROTOCOLS & WM_DELETE_WINDOW
  icccmAtoms[WmProtocols] = XInternAtom(
      display, "WM_PROTOCOLS", False);
  icccmAtoms[WmDeleteWindow] = XInternAtom(
      display, "WM_DELETE_WINDOW", False);

  // Setup EWMH Atoms for:
  // _NET_WM_SUPPORTED & _NET_ACTIVE_WINDOW
  ewmhAtoms[NetSupported] = XInternAtom(
      display, "_NET_SUPPORTED", False);
  ewmhAtoms[NetActive] = XInternAtom(
      display, "_NET_ACTIVE_WINDOW", False);

  XChangeProperty(display, root, ewmhAtoms[NetSupported], 
      XA_ATOM, 32, PropModeReplace, (unsigned char *) ewmhAtoms, NetCount);

  XSync(display, False);
}
