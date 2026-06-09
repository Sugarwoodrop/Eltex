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
#define CLI_PORT 33332
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *msg = argc > 1 ? argv[1] : "hello from raw ip+udp";
    size_t mlen = strlen(msg);

    uint32_t saddr = inet_addr(SRV_IP);
    uint32_t daddr = inet_addr(SRV_IP);

    int guard = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ga = {0};
    ga.sin_family = AF_INET; ga.sin_addr.s_addr = htonl(INADDR_ANY); ga.sin_port = htons(CLI_PORT);
    bind(guard, (struct sockaddr *)&ga, sizeof(ga));

    int raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw == ERROR) { perror("socket(SOCK_RAW) — нужен root"); exit(ERROR); }

    int one = 1;
    if (setsockopt(raw, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) == ERROR) {
        perror("IP_HDRINCL"); exit(ERROR);
    }
    struct timeval tv = { 3, 0 };
    setsockopt(raw, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t packet[2048];
    struct iphdr  *ip  = (struct iphdr *)packet;
    struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct iphdr));
    size_t udplen = sizeof(struct udphdr) + mlen;
    size_t total  = sizeof(struct iphdr) + udplen;

    memset(ip, 0, sizeof(struct iphdr));
    ip->version  = 4;
    ip->ihl      = 5;
    ip->tos      = 0;
    ip->tot_len  = htons(total);
    ip->id       = htons(0x1234);
    ip->frag_off = 0;
    ip->ttl      = 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr    = saddr;
    ip->daddr    = daddr;
    ip->check    = 0;
    ip->check    = checksum16(ip, sizeof(struct iphdr));

    udp->source = htons(CLI_PORT);
    udp->dest   = htons(SRV_PORT);
    udp->len    = htons(udplen);
    udp->check  = 0;
    memcpy(packet + sizeof(struct iphdr) + sizeof(struct udphdr), msg, mlen);
    udp->check  = udp_checksum(saddr, daddr, udp, msg, mlen);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(SRV_PORT);
    dst.sin_addr.s_addr = daddr;

    if (sendto(raw, packet, total, 0, (struct sockaddr *)&dst, sizeof(dst)) == ERROR) {
        perror("sendto"); exit(ERROR);
    }
    printf("Sent (IP + UDP headers built by client): '%s'\n", msg);

    uint8_t rbuf[2048];
    while (1) {
        ssize_t n = recv(raw, rbuf, sizeof(rbuf), 0);
        if (n == ERROR) { fprintf(stderr, "Нет ответа (таймаут)\n"); break; }
        struct iphdr *rip = (struct iphdr *)rbuf;
        if (rip->protocol != IPPROTO_UDP) continue;
        int ihl = rip->ihl * 4;
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
