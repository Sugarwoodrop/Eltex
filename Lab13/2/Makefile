CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client

server: server.c protocol.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c protocol.h
	$(CC) $(CFLAGS) -o client client.c -lncurses -lpthread

clean:
	rm -f server client
