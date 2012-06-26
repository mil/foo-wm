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

void crawlContainer(Container * container, int level);
void dumpTree();
void handleCommand(char* request);
int parentClient(Client * child, Container * parent);
int parentContainer(Container * child, Container * parent);
int placeContainer(Container * container, int x, int y, int width, int height);
Client * getClientByWindow(Window * window);
unsigned long getColor(const char *colstr);
void centerPointer(Window *window);
void focusWindow(Window * window);
void handleXEvent(XEvent *event);
void handleEvents();
int xError(XErrorEvent *e);
