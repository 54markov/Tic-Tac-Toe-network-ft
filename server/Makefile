all: server

server: server.c implementation.o header.h
	gcc -g -o server server.c implementation.o -std=c11

implementation: implementation.c header.h
	gcc -c implementation.c -std=c11	

clean:
	rm -f *.o server