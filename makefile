CC = gcc
CFLAGS = -g -Wall -I.


y86emul: y86emul.c y86emul.o
	gcc -g -Wall -o y86emul y86emul.o -I.

clean:
	rm -f y86emul y86emul.o 