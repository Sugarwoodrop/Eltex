#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define GROUP "239.0.0.1"
#define PORT  6001
#define BUF   256
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *ifip = argc > 1 ? argv[1] : NULL;

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

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = ifip ? inet_addr(ifip) : htonl(INADDR_ANY);
    if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == ERROR) {
        perror("IP_ADD_MEMBERSHIP"); exit(ERROR);
    }
    printf("Multicast server joined %s:%d\n", GROUP, PORT);

    char buf[BUF];
    struct sockaddr_in cli;
    socklen_t clen = sizeof(cli);

    while (1) {
        ssize_t n = recvfrom(s, buf, BUF - 1, 0, (struct sockaddr *)&cli, &clen);
        if (n == ERROR) { perror("recvfrom"); break; }
        buf[n] = '\0';
        printf("From %s: %s\n", inet_ntoa(cli.sin_addr), buf);
    }

    setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    close(s);
    return 0;
}
