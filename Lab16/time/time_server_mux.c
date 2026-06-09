/*
  Вариант 4: мультипротокольный сервер. В одном потоке обслуживает
  и TCP, и UDP клиентов через мультиплексирование (epoll). 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define PORT  7000
#define MAXEV 32
#define ERROR -1

static void time_str(char *out, size_t n) {
    time_t now = time(NULL);
    snprintf(out, n, "%s", ctime(&now));
}

static int make_tcp(void) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_ANY); a.sin_port = htons(PORT);
    bind(s, (struct sockaddr *)&a, sizeof(a));
    listen(s, 16);
    return s;
}

static int make_udp(void) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_ANY); a.sin_port = htons(PORT);
    bind(s, (struct sockaddr *)&a, sizeof(a));
    return s;
}

int main() {
    int tcp = make_tcp();
    int udp = make_udp();
    if (tcp == ERROR || udp == ERROR) { perror("socket"); exit(ERROR); }

    int ep = epoll_create1(0);
    struct epoll_event ev;

    ev.events = EPOLLIN; ev.data.fd = tcp; epoll_ctl(ep, EPOLL_CTL_ADD, tcp, &ev);
    ev.events = EPOLLIN; ev.data.fd = udp; epoll_ctl(ep, EPOLL_CTL_ADD, udp, &ev);

    printf("Multiprotocol time server (TCP+UDP, epoll) on port %d\n", PORT);

    struct epoll_event events[MAXEV];
    char buf[256];

    while (1) {
        int n = epoll_wait(ep, events, MAXEV, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == tcp) {
                int c = accept(tcp, NULL, NULL);
                if (c != ERROR) {
                    recv(c, buf, sizeof(buf), 0);
                    time_str(buf, sizeof(buf));
                    write(c, buf, strlen(buf));
                    close(c);
                }
            } else if (fd == udp) {
                struct sockaddr_in cli;
                socklen_t cl = sizeof(cli);
                ssize_t r = recvfrom(udp, buf, sizeof(buf), 0, (struct sockaddr *)&cli, &cl);
                if (r > 0) {
                    time_str(buf, sizeof(buf));
                    sendto(udp, buf, strlen(buf), 0, (struct sockaddr *)&cli, cl);
                }
            }
        }
    }

    close(tcp); close(udp); close(ep);
    return 0;
}
