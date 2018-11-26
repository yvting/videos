CC=g++
SLD2=/usr/include/SDL2
LCURSES=-lncurses -std=c++11
LSDL2=-DUNICODE -I$(SDL2) -lSDL2 -lpthread -std=c++11
OBJS=tetris mazes

all: $(OBJS)
	@echo "Done."

tetris: OneLoneCoder_Tetris_linux.cpp
	$(CC) $< $(LCURSES) -o $@

mazes: OneLoneCoder_Mazes.cpp
	$(CC) $< $(LSDL2) -o $@

.PHONY: clean

clean:
	rm $(OBJS)
