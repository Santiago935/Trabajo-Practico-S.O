CC = gcc
CFLAGS = -Wall -pthread

all: Servidor Cliente

Servidor: Servidor.c
	$(CC) $(CFLAGS) Servidor.c -o Servidor

Cliente: Cliente.c
	$(CC) $(CFLAGS) Cliente.c -o Cliente

clean:
	rm -f Servidor Cliente
