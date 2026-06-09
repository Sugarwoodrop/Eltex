#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 7100
#define BUF  1024
#define ERROR -1

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
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
    printf("UDP server on port %d\n", PORT);

    char buf[BUF];
    struct sockaddr_in cli;
    socklen_t clen = sizeof(cli);

    while (1) {
        ssize_t n = recvfrom(s, buf, BUF - 1, 0, (struct sockaddr *)&cli, &clen);
        if (n == ERROR) { perror("recvfrom"); continue; }
        buf[n] = '\0';
        printf("From %s:%d -> '%s'\n",
               inet_ntoa(cli.sin_addr), ntohs(cli.sin_port), buf);

        int done = 0;
        for (ssize_t i = 0; i < n; i++) {
            if (isalpha((unsigned char)buf[i])) {
                buf[i] = islower((unsigned char)buf[i])
                         ? toupper((unsigned char)buf[i])
                         : tolower((unsigned char)buf[i]);
                done = 1;
                break;
            }
        }
        if (!done && n > 0) buf[0] = '*';

        printf("Modified -> '%s'\n", buf);
        sendto(s, buf, n, 0, (struct sockaddr *)&cli, clen);
    }

    close(s);
    return 0;
}
