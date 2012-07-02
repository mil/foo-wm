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

/* Struct used for looking up a window's associated client */
typedef struct Lookup Lookup;
struct Lookup {
	int window;
	Node *node;
	Lookup *previous;
};
