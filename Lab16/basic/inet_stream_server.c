#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define BUF  256
#define ERROR -1

int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == ERROR) {
        perror("bind"); exit(ERROR);
    }
    if (listen(s, 5) == ERROR) { perror("listen"); exit(ERROR); }
    printf("TCP/INET server on port %d\n", PORT);

    int c = accept(s, NULL, NULL);
    if (c == ERROR) { perror("accept"); exit(ERROR); }

    char buf[BUF];
    ssize_t n = recv(c, buf, BUF - 1, 0);
    if (n == ERROR) { perror("recv"); exit(ERROR); }
    buf[n] = '\0';
    printf("Received: %s\n", buf);

    const char *reply = "hi!";
    send(c, reply, strlen(reply), 0);

    close(c);
    close(s);
    return 0;
}
