CC = gcc
CFLAGS = -std=gnu99 -Wpedantic

all: myShell

myShell.o: myShell.c
	$(CC) $(CFLAGS) -c myShell.c -o myShell.o

myShell: myShell.o
	$(CC) $(CFLAGS) -o myShell myShell.o

clean:
	rm *.o myShell