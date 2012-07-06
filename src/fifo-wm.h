#include <X11/Xlib.h>
#include <sys/time.h>
#include "structs.h"


Lookup * lookup;

int screen, activeScreen;
int padding;
int layout;
int border;
Display	*display;
Window root; 
struct timeval tv;
long unfocusedColor;
long focusedColor; 

int rootX, rootY, rootWidth, rootHeight;

/* focusedNode  :: Currently focused client node (recieves input)
 * selectedNode :: Currently selected node (for containerize, move, etc....)
 * viewNode     :: Currently viewed node (screenspace) */
Node *focusedNode, *selectedNode, *viewNode; 

void handleCommand(char* request);
void handleEvents();
