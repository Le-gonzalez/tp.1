CFLAGS=-Wall -std=c99 -pedantic -g
LFLAGS=-lm -lSDL2 -lSDL2_ttf -DTTF

all: peggle

peggle: main.o obstaculo.o poligono.o lista.o 
	gcc  main.o obstaculo.o poligono.o lista.o -o peggle $(LFLAGS)

main.o: main.c obstaculo.h lista.h config.h
	gcc $(CFLAGS) -c main.c

obstaculo.o: obstaculo.c obstaculo.h poligono.h
	gcc $(CFLAGS) -c obstaculo.c

poligono.o: poligono.c poligono.h 
	gcc $(CFLAGS) -c poligono.c

lista.o: lista.c lista.h
	gcc $(CFLAGS) -c lista.c
	
clean: 
	rm -vf *.o peggle
