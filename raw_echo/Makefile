CC = gcc
CFLAGS = -Wall -Wextra -g

PROGS = raw_echo_server raw_echo_client

all: $(PROGS)

%: %.c raw_util.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
