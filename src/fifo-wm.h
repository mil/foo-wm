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
 * viewNode     :: Currently viewed node (screenspace) 
 * rootNode     :: Node at the top of the tree*/
Node *focusedNode, *selectedNode, *viewNode, *rootNode;

/* X Window -> Node Lookup Table */
Lookup * lookup;

//Customizations
int clientPadding, containerPadding;
int layout, border;
long unfocusedColor, focusedColor, selectedColor;


void handleCommand(char* request);
void handleEvents();
