#include <X11/Xlib.h>

typedef struct Client Client;
struct Client {
	char			*label; 
	Window    window; 

	int layout; 
	int width;
	int height;

	Client    *parent;
	Client    *child; 
	Client    *next;
	Client    *previous;
};
