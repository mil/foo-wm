#include <X11/Xlib.h>
#include "structs.h"

Container * currentContainer;
Container * lastContainer;
Lookup * lookup;

int screen, activeScreen;
int padding;
int layout;
Display	*display;
Window root; 
struct timeval tv;
long unfocusedColor;
long focusedColor; 

void handleCommand(char* request);
void handleEvents();
