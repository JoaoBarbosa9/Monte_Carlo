# Compiler: gcc for C files, g++ for c++
CC = g++

# Compiler flags:
CFLAGS = -g -std=c++11

#the executable
TARGET = main

main : new_main.cpp game_state.h bot_class.h
	$(CC) $(CFLAGS) -o main new_main.cpp
clean:
	$(RM) main *.o *ÂÂ~
