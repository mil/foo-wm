enum { WmProtocols, WmDeleteWindow, WmCount };
Atom icccmAtoms[WmCount];

void sendDeleteWindow(Window * window);
void setupAtoms(void);
