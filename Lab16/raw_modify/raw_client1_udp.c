#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/time.h>

#include "raw_util.h"

#define SRV_IP   "127.0.0.1"
#define SRV_PORT 7100
#define CLI_PORT 33331
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *msg = argc > 1 ? argv[1] : "hello from raw udp";
    size_t mlen = strlen(msg);

    uint32_t saddr = inet_addr(SRV_IP);
    uint32_t daddr = inet_addr(SRV_IP);

    int guard = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ga = {0};
    ga.sin_family = AF_INET;
    ga.sin_addr.s_addr = htonl(INADDR_ANY);
    ga.sin_port = htons(CLI_PORT);
    bind(guard, (struct sockaddr *)&ga, sizeof(ga));

    int raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw == ERROR) { perror("socket(SOCK_RAW) — нужен root"); exit(ERROR); }

    struct timeval tv = { 3, 0 };
    setsockopt(raw, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t packet[2048];
    struct udphdr *udp = (struct udphdr *)packet;
    udp->source = htons(CLI_PORT);
    udp->dest   = htons(SRV_PORT);
    udp->len    = htons(sizeof(struct udphdr) + mlen);
    udp->check  = 0;
    memcpy(packet + sizeof(struct udphdr), msg, mlen);
    udp->check  = udp_checksum(saddr, daddr, udp, msg, mlen);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(SRV_PORT);
    dst.sin_addr.s_addr = daddr;

    size_t plen = sizeof(struct udphdr) + mlen;
    if (sendto(raw, packet, plen, 0, (struct sockaddr *)&dst, sizeof(dst)) == ERROR) {
        perror("sendto"); exit(ERROR);
    }
    printf("Sent (UDP header built by client): '%s'\n", msg);

    uint8_t rbuf[2048];
    while (1) {
        ssize_t n = recv(raw, rbuf, sizeof(rbuf), 0);
        if (n == ERROR) { fprintf(stderr, "Нет ответа (таймаут)\n"); break; }

        struct iphdr *ip = (struct iphdr *)rbuf;
        if (ip->protocol != IPPROTO_UDP) continue;
        int ihl = ip->ihl * 4;
        struct udphdr *ru = (struct udphdr *)(rbuf + ihl);

        if (ru->source != htons(SRV_PORT) || ru->dest != htons(CLI_PORT)) continue;

        char *pl = (char *)(rbuf + ihl + sizeof(struct udphdr));
        int pllen = ntohs(ru->len) - sizeof(struct udphdr);
        pl[pllen] = '\0';
        printf("Server replied: '%s'\n", pl);
        break;
    }

    close(raw);
    close(guard);
    return 0;
}
