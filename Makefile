minivim: minivim.o buffer.o regex.o
	g++ -Wall minivim.o buffer.o regex.o -o minivim -lncurses

minivim.o: minivim.c minivim.h buffer.h
	g++ -Wall -c minivim.c -o minivim.o

buffer.o: buffer.h buffer.c
	g++ -Wall -c buffer.c -o buffer.o

regex.o: regex.h buffer.h regex.cpp
	g++ -Wall -c regex.cpp -o regex.o

.PHONY: clean

clean:
	rm minivim *.o
