CFLAGS = -I ./include
##LIB    = ./libggfonts.so
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr

all: hw1

hw1: hw1.cpp
	g++ $(CFLAGS) hw1.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -o hw1

clean:
	rm -f hw1
	rm -f *.o

