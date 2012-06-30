void crawlContainer(Container * container, int level);
void dumpTree();
int parentClient(Client * child, Container * parent);
void destroyClient(Client * c);
void destroyContainer(Container * c);
int parentContainer(Container * child, Container * parent);
int placeContainer(Container * container, int x, int y, int width, int height);
Client * getClientByWindow(Window * window);
