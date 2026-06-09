CC = gcc
CFLAGS = -Wall -Wextra -g

all: sig_receiver sig_sender sig_block sig_wait

sig_receiver: sig_receiver.c
	$(CC) $(CFLAGS) -o sig_receiver sig_receiver.c

sig_sender: sig_sender.c
	$(CC) $(CFLAGS) -o sig_sender sig_sender.c

sig_block: sig_block.c
	$(CC) $(CFLAGS) -o sig_block sig_block.c

sig_wait: sig_wait.c
	$(CC) $(CFLAGS) -o sig_wait sig_wait.c

clean:
	rm -f sig_receiver sig_sender sig_block sig_wait
