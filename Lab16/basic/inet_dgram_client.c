#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5000
#define BUF  256
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *ip = argc > 1 ? argv[1] : "127.0.0.1";

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    inet_pton(AF_INET, ip, &srv.sin_addr);

    const char *msg = "hello!";
    sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&srv, sizeof(srv));

    char buf[BUF];
    ssize_t n = recvfrom(s, buf, BUF - 1, 0, NULL, NULL);
    if (n == ERROR) { perror("recvfrom"); exit(ERROR); }
    buf[n] = '\0';
    printf("Server replied: %s\n", buf);

    close(s);
    return 0;
}
