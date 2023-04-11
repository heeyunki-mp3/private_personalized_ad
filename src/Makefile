# Makefile for test program

CXX = g++
CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -g

test: test.o user.o ad.o

test.o: test.cpp

user.o: user.cpp

ad.o: ad.cpp

.PHONY: clean
clean:
	rm -rf *.o test

.PHONY: all
all: clean test
