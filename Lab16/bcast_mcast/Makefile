CC = gcc
CFLAGS = -Wall -Wextra -g

PROGS = bcast_server bcast_client mcast_server mcast_client

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
