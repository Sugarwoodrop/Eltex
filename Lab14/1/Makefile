CC = gcc
CFLAGS = -Wall -Wextra -g

all: posix_server posix_client sysv_server sysv_client

posix_server: posix_server.c
	$(CC) $(CFLAGS) -o posix_server posix_server.c -lrt -lpthread

posix_client: posix_client.c
	$(CC) $(CFLAGS) -o posix_client posix_client.c -lrt -lpthread

sysv_server: sysv_server.c
	$(CC) $(CFLAGS) -o sysv_server sysv_server.c

sysv_client: sysv_client.c
	$(CC) $(CFLAGS) -o sysv_client sysv_client.c

clean:
	rm -f posix_server posix_client sysv_server sysv_client
