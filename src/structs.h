#include <X11/Xlib.h>


typedef struct Client Client;
typedef struct Container Container;

struct Client {
	Window    window; 

	Container *parent;
	Client    *next;
	Client    *previous;
};
struct Container {
	char			*label; 
	int       layout; 
	int				x, y, width, height;
	Client    *focus;

	Container *parent;
	Container *next;
	Container *previous;

	/* Container will only ever have one of these at a time */
	Container *child;
	Client    *client;
};

/* Struct used for looking up a window's associated client */
typedef struct Lookup Lookup;
struct Lookup {
	int window;
	Client *client;
	Lookup *previous;
};
