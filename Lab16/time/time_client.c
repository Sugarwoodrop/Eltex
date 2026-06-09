#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 7000
#define BUF  256
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *host = argc > 1 ? argv[1] : "127.0.0.1";
    int port = argc > 2 ? atoi(argv[2]) : PORT;
    int udp  = (argc > 3 && strcmp(argv[3], "udp") == 0);

    int s = socket(AF_INET, udp ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    inet_pton(AF_INET, host, &srv.sin_addr);

    char buf[BUF];
    if (udp) {
        const char *req = "TIME";
        sendto(s, req, strlen(req), 0, (struct sockaddr *)&srv, sizeof(srv));
        ssize_t n = recvfrom(s, buf, BUF - 1, 0, NULL, NULL);
        if (n == ERROR) { perror("recvfrom"); exit(ERROR); }
        buf[n] = '\0';
    } else {
        if (connect(s, (struct sockaddr *)&srv, sizeof(srv)) == ERROR) {
            perror("connect"); exit(ERROR);
        }
        const char *req = "TIME";
        send(s, req, strlen(req), 0);
        ssize_t n = recv(s, buf, BUF - 1, 0);
        if (n == ERROR) { perror("recv"); exit(ERROR); }
        buf[n] = '\0';
    }

    printf("Server time: %s", buf);
    close(s);
    return 0;
}
