#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define BUF  256
#define ERROR -1

int main() {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == ERROR) {
        perror("bind"); exit(ERROR);
    }
    printf("UDP/INET server on port %d\n", PORT);

    struct sockaddr_in cli;
    socklen_t clen = sizeof(cli);
    char buf[BUF];

    ssize_t n = recvfrom(s, buf, BUF - 1, 0, (struct sockaddr *)&cli, &clen);
    if (n == ERROR) { perror("recvfrom"); exit(ERROR); }
    buf[n] = '\0';
    printf("Received: %s\n", buf);

    const char *reply = "hi!";
    sendto(s, reply, strlen(reply), 0, (struct sockaddr *)&cli, clen);

    close(s);
    return 0;
}
