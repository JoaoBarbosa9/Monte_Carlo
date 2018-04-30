# Compiler: gcc for C files, g++ for c++
CC = g++

# Compiler flags:
CFLAGS = -g -std=c++11

#the executable
TARGET = main

main : main.cpp game_state.h
	$(CC) $(CFLAGS) -o main main.cpp
clean:
	$(RM) main *.o *ÂÂ~
