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
#define SRV_PORT 7200
#define OP_DATA  1
#define OP_CLOSE 2
#define ERROR -1

static int guard_fd, raw_fd;
static uint32_t saddr, daddr;
static uint16_t cli_port;

static void send_payload(uint8_t op, const char *text) {
    uint8_t packet[2048];
    struct udphdr *udp = (struct udphdr *)packet;
    size_t tlen = text ? strlen(text) : 0;
    size_t plen = 1 + tlen;
    uint8_t *pl = packet + sizeof(struct udphdr);
    pl[0] = op;
    if (tlen) memcpy(pl + 1, text, tlen);

    udp->source = htons(cli_port);
    udp->dest   = htons(SRV_PORT);
    udp->len    = htons(sizeof(struct udphdr) + plen);
    udp->check  = 0;
    udp->check  = udp_checksum(saddr, daddr, udp, pl, plen);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(SRV_PORT);
    dst.sin_addr.s_addr = daddr;
    sendto(raw_fd, packet, sizeof(struct udphdr) + plen, 0,
           (struct sockaddr *)&dst, sizeof(dst));
}

static int recv_reply(char *out, size_t outsz) {
    uint8_t rbuf[2048];
    while (1) {
        ssize_t n = recv(raw_fd, rbuf, sizeof(rbuf), 0);
        if (n == ERROR) return -1;
        struct iphdr *ip = (struct iphdr *)rbuf;
        if (ip->protocol != IPPROTO_UDP) continue;
        int ihl = ip->ihl * 4;
        struct udphdr *udp = (struct udphdr *)(rbuf + ihl);
        if (udp->source != htons(SRV_PORT) || udp->dest != htons(cli_port)) continue;
        uint8_t *pl = rbuf + ihl + sizeof(struct udphdr);
        int pllen = ntohs(udp->len) - sizeof(struct udphdr);
        if (pllen < 1) continue;
        int tlen = pllen - 1;
        if (tlen > (int)outsz - 1) tlen = outsz - 1;
        memcpy(out, pl + 1, tlen);
        out[tlen] = '\0';
        return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <client_port> <msg|--close> [msg2 ...]\n", argv[0]);
        return ERROR;
    }
    cli_port = (uint16_t)atoi(argv[1]);
    saddr = inet_addr(SRV_IP);
    daddr = inet_addr(SRV_IP);

    guard_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ga = {0};
    ga.sin_family = AF_INET; ga.sin_addr.s_addr = htonl(INADDR_ANY); ga.sin_port = htons(cli_port);
    if (bind(guard_fd, (struct sockaddr *)&ga, sizeof(ga)) == ERROR) {
        perror("bind guard"); return ERROR;
    }

    raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw_fd == ERROR) { perror("socket(SOCK_RAW) — нужен root"); return ERROR; }
    struct timeval tv = { 3, 0 };
    setsockopt(raw_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--close") == 0) {
            send_payload(OP_CLOSE, NULL);
            printf("[port %d] sent CLOSE\n", cli_port);
            continue;
        }
        send_payload(OP_DATA, argv[i]);
        char reply[1024];
        if (recv_reply(reply, sizeof(reply)) == 0)
            printf("[port %d] sent '%s' -> reply '%s'\n", cli_port, argv[i], reply);
        else
            printf("[port %d] sent '%s' -> нет ответа (таймаут)\n", cli_port, argv[i]);
    }

    close(raw_fd);
    close(guard_fd);
    return 0;
}
