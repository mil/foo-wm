#include <X11/Xlib.h>
#include "structs.h"

Container * rootContainer;
Container * currentContainer;
Client * currentClient;
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

void handleCommand(char* request);
void handleEvents();
