PrismCore: main.c structures.o hardwareRender.o glad.o
	gcc -g main.c structures.o hardwareRender.o glad.o   -o PrismCore   -I./include -L./lib -lopengl32 -lSDL3
	make clean

PrismCoreSoftware: main-software.c structures.o softwareRender.o
	gcc -g main-software.c structures.o softwareRender.o   -o PrismCoreSoftware   -I./include -L./lib -lSDL3
	make clean




structures.o: structures.c 
	gcc -c structures.c -Iinclude -Llib -lSDL3

hardwareRender.o: hardwareRender.c
	gcc -c hardwareRender.c -Iinclude -Llib -lopengl32 -lSDL3

softwareRender.o: softwareRender.c
	gcc -c softwareRender.c -Iinclude -Llib -lSDL3

glad.o: glad.c
	gcc -c glad.c -Iinclude -Llib -lopengl32 -lSDL3




clean:
	rm *.o