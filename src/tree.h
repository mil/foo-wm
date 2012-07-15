void crawlNode(Node * node, int level);
void dumpTree();
void destroyNode(Node * n);
void unparentNode(Node * node);
void brotherNode(Node *node, Node * brother, int position);
void parentNode(Node * node, Node * parent);
void focusNode(Node * node, XEvent * event);
void selectNode(Node * n, Bool setSelected);
void unmapNode(Node * node);
void placeNode(Node * node, int x, int y, int width, int height);
Node * getBrotherClient(Node * node, int direction);
Node * getClosestClient(Node * node);

Bool isClient(Node * n);
