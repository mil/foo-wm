#include <X11/Xlib.h>
#include <sys/time.h>
#include "structs.h"

//X Server
Display *display;
int screen, activeScreen;
int rootX, rootY, rootWidth, rootHeight;
Window root; 

/* focusedNode  :: Currently focused node (innard most of fcs ptrs rcvs input)
 * viewNode     :: Currently viewed node (screenspace) 
 * rootNode     :: Node at the top of the tree*/
Node *focusedNode, *viewNode, *rootNode;

/* X Window -> Node Lookup Table */
Lookup * lookup;
Mark   * mark;

//Customizations
int clientPadding, containerPadding;
int defaultLayout, border;
long activeFocusedColor, activeUnfocusedColor, 
     inactiveFocusedColor, inactiveUnfocusedColor;

void setup(void);
void handleEvents(void);
