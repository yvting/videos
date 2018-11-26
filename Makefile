CC=g++
LCURSES=-lncurses
LSDL2=-DUNICODE -I/usr/include/SDL2 -lSDL2 -lpthread -std=c++11
OBJS=tetris mazes

all: $(OBJS)
	@echo "Done."

tetris: OneLoneCoder_Tetris_linux.cpp
	$(CC) $< $(LCURSES) -o $@

mazes: OneLoneCoder_Mazes.cpp
	$(CC) $< $(LSDL2) -o $@
