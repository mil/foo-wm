unsigned long getColor(const char *colstr);
int xError(XErrorEvent *e);
Node * allocateNode();
void gridDimensions(int children, int * rows, int * cols);
Bool isPrime(int number);
Node * focusOrChildOf(Node * node);
void recalculateRootDimensions (void);
int bytesUntilNull(char * string);
void stringAppend(char ** originalString, char * appendContent);
