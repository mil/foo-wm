CC=gcc
PRECFLAGS=-Wall -g
POSTCFLAGS=-lX11 -lm
VPATH=./src

WM_OBJS= atoms.o events.o tree.o commands.o responses.o util.o window.o lookup.o foo-wm.o
CLI_OBJS= client.o

all: foo-wm foo-wm-c

src/config.h:
	cp src/config.def.h src/config.h

foo-wm: src/config.h $(WM_OBJS)
	$(CC) $(PRECFLAGS) $(WM_OBJS) $(POSTCFLAGS) -o foo-wm

foo-wm-c: $(CLI_OBJS)
	$(CC) $(PRECFLAGS) $(CLI_OBJS) $(POSTCFLAGS) -o foo-wm-c

clean:
	rm -rf foo-wm foo-wm-c *.o
