CC = gcc
CFLAGS = -Wall -Wextra -g

PROGS = inet_dgram_server inet_dgram_client \
        inet_stream_server inet_stream_client \
        local_dgram_server local_dgram_client \
        local_stream_server local_stream_client

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
