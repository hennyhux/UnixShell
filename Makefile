CC=gcc
OUTPUT=myshell
SOURCE=myshell.c

all:
	$(CC) -g -o $(OUTPUT) $(SOURCE)

