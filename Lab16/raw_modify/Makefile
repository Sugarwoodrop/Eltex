CC = gcc
CFLAGS = -Wall -Wextra -g

PROGS = udp_server raw_client1_udp raw_client2_ip raw_client3_eth

all: $(PROGS)

%: %.c raw_util.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
