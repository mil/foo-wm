void crawlNode(Node * node, int level);
void dumpTree();
void destroyNode(Node * n);
void unparentNode(Node * node);
void parentNode(Node * node, Node * parent);
void focusNode(Node * node);
void unmapNode(Node * node);
void placeNode(Node * node, int x, int y, int width, int height);
Node * getBrotherClient(Node * node, int direction);
Node * getClosestClient(Node * node);

Bool isClient(Node * n);
