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


void setupAtoms(void) {
  // Set Up ICCCM Atoms for:
  // WM_PROTOCOLS & WM_DELETE_WINDOW
  icccmAtoms[WmProtocols] = XInternAtom(
      display, "WM_PROTOCOLS", False);
  icccmAtoms[WmDeleteWindow] = XInternAtom(
      display, "WM_DELETE_WINDOW", False);

  XSync(display, False);
}
