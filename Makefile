minivim: minivim.o buffer.o
	gcc -Wall minivim.o buffer.o -o minivim -lncurses

minivim.o: minivim.c minivim.h buffer.h
	gcc -Wall -c minivim.c -o minivim.o

buffer.o: buffer.h buffer.c
	gcc -Wall -c buffer.c -o buffer.o

.PHONY: clean

clean:
	rm minivim *.o
