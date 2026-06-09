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

    unlink(SRV_PATH);
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SRV_PATH, sizeof(addr.sun_path) - 1);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == ERROR) {
        perror("bind"); exit(ERROR);
    }
    if (listen(s, 5) == ERROR) { perror("listen"); exit(ERROR); }
    printf("LOCAL/STREAM server on %s\n", SRV_PATH);

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
    unlink(SRV_PATH);
    return 0;
}
