#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SRV_PATH "/tmp/local_dgram_srv"
#define CLI_PATH "/tmp/local_dgram_cli"
#define BUF  256
#define ERROR -1

int main() {
    int s = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    unlink(CLI_PATH);
    struct sockaddr_un me = {0};
    me.sun_family = AF_LOCAL;
    strncpy(me.sun_path, CLI_PATH, sizeof(me.sun_path) - 1);
    if (bind(s, (struct sockaddr *)&me, sizeof(me)) == ERROR) {
        perror("bind"); exit(ERROR);
    }

    struct sockaddr_un srv = {0};
    srv.sun_family = AF_LOCAL;
    strncpy(srv.sun_path, SRV_PATH, sizeof(srv.sun_path) - 1);

    const char *msg = "hello!";
    sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&srv, sizeof(srv));

    char buf[BUF];
    ssize_t n = recvfrom(s, buf, BUF - 1, 0, NULL, NULL);
    if (n == ERROR) { perror("recvfrom"); exit(ERROR); }
    buf[n] = '\0';
    printf("Server replied: %s\n", buf);

    close(s);
    unlink(CLI_PATH);
    return 0;
}
