#include <X11/Xlib.h>
#include <sys/time.h>
#include "structs.h"


//X Server
Display	*display;
int screen, activeScreen;
int rootX, rootY, rootWidth, rootHeight;
Window root; 

/* focusedNode  :: Currently focused client node (recieves input)
 * selectedNode :: Currently selected node (for containerize, move, etc....)
 * viewNode     :: Currently viewed node (screenspace) */
Node *focusedNode, *selectedNode, *viewNode; 

/* X Window -> Node Lookup Table */
Lookup * lookup;

//Customizations
int padding, layout, border;
long unfocusedColor, focusedColor; 


void handleCommand(char* request);
void handleEvents();
