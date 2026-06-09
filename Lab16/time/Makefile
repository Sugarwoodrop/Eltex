CC = gcc
CFLAGS = -Wall -Wextra -g

CLIENT = time_client
SERVERS = time_server_thread time_server_pool time_server_prodcons time_server_mux

all: $(CLIENT) $(SERVERS)

$(CLIENT): $(CLIENT).c
	$(CC) $(CFLAGS) -o $@ $<

time_server_mux: time_server_mux.c
	$(CC) $(CFLAGS) -o $@ $<

time_server_%: time_server_%.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

clean:
	rm -f $(CLIENT) $(SERVERS)
