all: main

main: ../lib/graphics/myBigChars.o ../lib/graphics/myReadKey.o ../lib/graphics/myTerm.o
	gcc -g -o client client.c logic.c ../lib/graphics/myBigChars.o ../lib/graphics/myReadKey.o ../lib/graphics/myTerm.o -lpthread 
	
graphics: ../lib/graphics/myBigChars.c ../lib/graphics/myReadKey.c ../lib/graphics/myTerm.c
	gcc -c ../lib/graphics/myBigChars.c ../lib/graphics/myReadKey.c ../lib/graphics/myTerm.c

clean:
	rm -f *.o client