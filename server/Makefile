# Makefile for server

# Makefile for user

CXX = g++
CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -g

server: server.o

server.o: server.cpp

.PHONY: clean
clean:
	rm -rf *.o server

.PHONY: all
all: clean server
