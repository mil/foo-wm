#include <X11/Xlib.h>

typedef struct Client Client;
struct Client {
	Window window;
	Client *previous;
	Client *next;
};

typedef struct Container Container;
struct Container {
	Client    *lastClient;
	Container *lastContainer; 
	Client    *previous; //Previous Container in stack
	char			*label; //Name of Containers Label
};


typedef struct {
	Window active;
	Client *last;
} Workspace;
