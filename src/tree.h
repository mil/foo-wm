Bool isClient(Node * node);
Bool isOnlyChild(Node * node);
Bool nodeIsParentOf(Node * nodeA, Node * nodeB);
Bool unfocusNode(Node * n, Bool focusPath);

char * crawlNode(Node * node, int level);
long getBorderColor(Node * node, Bool focusPath);

Node * getBrother(Node * node, int delta);
Node * getBrotherClient(Node * node, int direction);
Node * getClosestClient(Node * node);

void brotherNode(Node *node, Node * brother, int position);
void destroyNode(Node * n);
void dumpTree();
void focusNode(Node * n, XEvent * event, Bool setFocused, Bool focusPath);
void parentNode(Node *node, Node *parent);
void placeNode(Node * node, int x, int y, int width, int height);
void rePlaceNode(Node * node);
void swapNodes(Node * a, Node * b);
void unmapNode(Node * node);
void unparentNode(Node *node);
