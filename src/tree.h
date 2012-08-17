void crawlNode(Node * node, int level);
void dumpTree();
void destroyNode(Node * n);
void unparentNode(Node * node);
void brotherNode(Node *node, Node * brother, int position);
void parentNode(Node * node, Node * parent);
Bool unfocusNode(Node * n, Bool focusPath);
void focusNode(Node * node, XEvent * event, Bool setFocused, Bool focusPath);
void unmapNode(Node * node);
void placeNode(Node * node, int x, int y, int width, int height);
void swapNodes(Node * a, Node * b);

Node * getBrother(Node * origin, int delta);
Node * getClientByDirection(Node * originNode, int direction);

Bool nodeIsParentOf(Node * nodeA, Node * nodeB);
Node * getBrotherByDirection(Node * node, int direction);
Node * getBrotherClient(Node * node, int direction);
Node * getClosestClient(Node * node);

Bool isClient(Node * n);
Bool isOnlyChild(Node * n);
