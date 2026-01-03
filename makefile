Program: main.c structures.o
	gcc main.c structures.o -o Program -lSDL3
	rm structures.o
	# gcc main.c -o Program (-Llib -Iinclude) -lSDL3

structures.o: structures.c 
	gcc -c structures.c