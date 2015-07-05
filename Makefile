CFLAGS = -ansi -pedantic -Wall -std=c++0x -w

all: flex bison
	g++ $(CFLAGS) src/tokens.cpp src/parser.cpp -o bin/lc3c -I ./src
	

flex: mkdir
	flex -o src/tokens.cpp src/lc3c.l

bison: mkdir
	bison -d -o src/parser.cpp src/lc3c.y

mkdir:
	mkdir -p bin
	mkdir -p ccode
