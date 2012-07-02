#include <X11/Xlib.h>
typedef struct Node Node;
struct Node {
	int layout;
	int x, y, width, height;

	Node *parent;
	Node *next;
	Node *previous;

	/* If a Container */
	Node *child;
	Node *focus;

	/* If a Client */
	Window window;
};


typedef struct Client Client;
typedef struct Container Container;

struct Client {
	Window    window; 
	int				x, y, width, height;

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
	Node *node;
	Lookup *previous;
};
