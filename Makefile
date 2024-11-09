#
# A simple makefile for compiling a c++ project
#
.DEFAULT_GOAL := myfind.cpp

CC = g++
CFLAGS = -Wall -g

all: clean myfind

myfind: myfind.o
	$(CC) $(CFLAGS) myfind.o -o myfind

myfind.o:
	$(CC) $(CFLAGS) -c myfind.cpp

clean: 
	rm -rf *.o myfind