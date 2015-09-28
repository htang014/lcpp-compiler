CFLAGS = -ansi -pedantic -Wall -std=c++11 -w

all: flex bison
	g++ $(CFLAGS) src/tokens.cpp src/parser.cpp -o bin/lc3c -I ./src
	

flex: mkdir
	flex -t src/lc3c.l > src/tokens.cpp

bison: mkdir
	bison -d -o src/parser.cpp src/lc3c.y

mkdir:
	mkdir -p bin
	mkdir -p ccode
