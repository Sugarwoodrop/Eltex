CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client

server: server.c shm_chat.h
	$(CC) $(CFLAGS) -o server server.c -lrt -lpthread

client: client.c shm_chat.h
	$(CC) $(CFLAGS) -o client client.c -lncurses -lpthread -lrt

clean:
	rm -f server client
