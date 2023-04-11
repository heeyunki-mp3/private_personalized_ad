# Makefile

CXX = g++
CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wshadow -g

test: server user ad

server.o: server.cpp

user.o: user.cpp

ad.o: ad.cpp

.PHONY: clean
clean:
	rm -rf *.o test

.PHONY: all
all: clean test
