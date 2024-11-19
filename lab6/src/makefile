CC = gcc
CFLAGS = -I. -lpthread

new: client_2 server_2

client_2: client_2.c MultModulo.o
	$(CC) -o client_2 client_2.c MultModulo.o $(CFLAGS)

server_2: server_2.c MultModulo.o
	$(CC) -o server_2 server_2.c MultModulo.o $(CFLAGS)

MultModulo.o: MultModulo.c MultModulo.h
	$(CC) -c MultModulo.c $(CFLAGS)

clean:
	rm -f client_2 server_2 MultModulo.o
