.DEFAULT_GOAL := myfind.cpp

CC = g++
CFLAGS = -Wall -g

all: clean myfind

myfind: main.o myfind.o
	$(CC) $(CFLAGS) main.o myfind.o -o myfind

main.o: src/main.cpp
	$(CC) $(CFLAGS) -c src/main.cpp

myfind.o: src/myfind.cpp include/myfind.h
	$(CC) $(CFLAGS) -c src/myfind.cpp

clean: 
	rm -rf *.o myfind