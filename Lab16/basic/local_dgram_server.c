#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SRV_PATH "/tmp/local_dgram_srv"
#define BUF  256
#define ERROR -1

int main() {
    int s = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    unlink(SRV_PATH);
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SRV_PATH, sizeof(addr.sun_path) - 1);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == ERROR) {
        perror("bind"); exit(ERROR);
    }
    printf("LOCAL/DGRAM server on %s\n", SRV_PATH);

    struct sockaddr_un cli;
    socklen_t clen = sizeof(cli);
    char buf[BUF];

    ssize_t n = recvfrom(s, buf, BUF - 1, 0, (struct sockaddr *)&cli, &clen);
    if (n == ERROR) { perror("recvfrom"); exit(ERROR); }
    buf[n] = '\0';
    printf("Received: %s\n", buf);

    const char *reply = "hi!";
    sendto(s, reply, strlen(reply), 0, (struct sockaddr *)&cli, clen);

    close(s);
    unlink(SRV_PATH);
    return 0;
}
