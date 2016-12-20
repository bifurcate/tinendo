CC = gcc

SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

CFLAGS =-std=c99 -g
LDFLAGS = $(SDL_LDFLAGS)

all: romtool

test: test.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) test.c -o test $(SDL_LDFLAGS)

romtool: romdumper.c
	$(CC) $(SDL_CFLAGS) romdumper.c -o romtool $(SDL_LDFLAGS)

emutest: main.o 6502.o nesmem.o 6502.h nesmem.h 
	$(CC) $(CFLAGS) -o $@ main.o 6502.o nesmem.o

main.o: main.c 6502.h nesmem.h
	$(CC) $(CFLAGS) -c -o $@ main.c

6502.o: 6502.c 6502.h nesmem.h
	$(CC) $(CFLAGS) -c -o $@ 6502.c

nesmem.o: nesmem.c nesmem.h 2c02.h
	$(CC) $(CFLAGS) -c -o $@ nesmem.c

clean:
	rm *.o hello

