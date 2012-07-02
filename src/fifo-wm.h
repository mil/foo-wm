#include <X11/Xlib.h>
#include "structs.h"

Container * rootContainer;
Container * currentContainer;
Lookup * lookup;

int screen, activeScreen;
int padding;
int layout;
int spawn;
Display	*display;
Window root; 
struct timeval tv;
long unfocusedColor;
long focusedColor; 

Node *activeNode; //Active Input / New clients launch as brother
Node *viewNode; // Currently viewed node on the screen

void handleCommand(char* request);
void handleEvents();
