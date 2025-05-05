CC = gcc
CFLAGS = -Wall -Wextra
EXEC = main
OBJS = src/main.c

all:
	$(CC) -o $(EXEC) $(OBJS)