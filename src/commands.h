char * handleCommand(char * request);
char * nextToken(char ** tokenString);

void containerize(void);
void focus(char * argA, char * argB);
void kill(void);
void jump(char * markName);
void layout(char * l);
void mark(char * markName);
void shift(int amount);
void zoom(int level);
