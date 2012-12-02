enum { WmProtocols, WmDeleteWindow, WmCount };
enum { NetSupported, NetActive, NetCount };
Atom icccmAtoms[WmCount];
Atom ewmhAtoms [NetCount];

void sendDeleteWindow(Window * window);
void setupAtoms(void);
