#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SRV_PATH "/tmp/local_stream_srv"
#define BUF  256
#define ERROR -1

int main() {
    int s = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    struct sockaddr_un srv = {0};
    srv.sun_family = AF_LOCAL;
    strncpy(srv.sun_path, SRV_PATH, sizeof(srv.sun_path) - 1);

    if (connect(s, (struct sockaddr *)&srv, sizeof(srv)) == ERROR) {
        perror("connect"); exit(ERROR);
    }

    const char *msg = "hello!";
    send(s, msg, strlen(msg), 0);

    char buf[BUF];
    ssize_t n = recv(s, buf, BUF - 1, 0);
    if (n == ERROR) { perror("recv"); exit(ERROR); }
    buf[n] = '\0';
    printf("Server replied: %s\n", buf);

    close(s);
    return 0;
}
