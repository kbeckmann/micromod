
CC=gcc
CFLAGS=-ansi -pedantic -Wall -ggdb -O0

all: micromod-sdl micromod-wav

clean:
	rm -f micromod micromod.exe stdout.txt stderr.txt *~

micromod-wav:
	$(CC) $(CFLAGS) micromod.c wavplayer.c -o micromod-wav

micromod-sdl:
	$(CC) $(CFLAGS) micromod.c sdlplayer.c -o micromod `sdl-config --cflags --libs`

micromod-w32:
	$(CC) $(CFLAGS) micromod.c w32player.c -o micromod -lwinmm

