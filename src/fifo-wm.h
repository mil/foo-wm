#include <X11/Xlib.h>
#include "structs.h"

Lookup * lookup;

int screen, activeScreen;
int padding;
int layout;
int spawn;
int border;
Display	*display;
Window root; 
struct timeval tv;
long unfocusedColor;
long focusedColor; 

int rootX, rootY, rootWidth, rootHeight;

Node *activeNode; //Active Input / New clients launch as brother
Node *viewNode; // Currently viewed node on the screen

void handleCommand(char* request);
void handleEvents();
