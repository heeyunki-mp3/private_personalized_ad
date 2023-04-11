# Makefile for user

CXX = g++
CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -g

user: user.o

user.o: user.cpp

.PHONY: clean
clean:
	rm -rf *.o user

.PHONY: all
all: clean user
