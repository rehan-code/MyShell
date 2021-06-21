CC = gcc
CFLAGS = -std=gnu99 -Wpedantic

all: myShell

myShell.o: myShell.c
	$(CC) $(CFLAGS) -c myShell.c -o myShell.o

myShell: myShell.o
	$(CC) $(CFLAGS) -o myShell myShell.o

bgSleep.o: bgSleep.c
	$(CC) $(CFLAGS) -c bgSleep.c -o bgSleep.o

bgSleep: bgSleep.o
	$(CC) $(CFLAGS) -o bgSleep bgSleep.o

clean:
	rm *.o myShell bgSleep