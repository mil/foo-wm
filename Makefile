CC=gcc
CFLAGS=-Wall -lX11 -g -lm
VPATH=./src

all:fifo-wm
fifo-wm: events.o tree.o commands.o util.o window.o lookup.o

clean:
	rm -rf fifo-wm *.o
