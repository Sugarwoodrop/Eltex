#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "raw_util.h"

#define SRV_PORT 7200
#define MAX_CLIENTS 64
#define MAX_TEXT 512
#define ERROR -1

#define OP_DATA  1
#define OP_CLOSE 2

typedef struct {
    uint32_t ip;
    uint16_t port;
    long     count;
    int      active;
} client_t;

static client_t clients[MAX_CLIENTS];

static client_t *find_client(uint32_t ip, uint16_t port) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].active && clients[i].ip == ip && clients[i].port == port)
            return &clients[i];
    return NULL;
}

static client_t *add_client(uint32_t ip, uint16_t port) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (!clients[i].active) {
            clients[i].active = 1;
            clients[i].ip = ip;
            clients[i].port = port;
            clients[i].count = 0;
            return &clients[i];
        }
    return NULL;
}

static void send_reply(int raw, uint32_t saddr, uint32_t daddr,
                       uint16_t sport, uint16_t dport,
                       const void *payload, size_t plen) {
    uint8_t packet[2048];
    struct udphdr *udp = (struct udphdr *)packet;
    udp->source = htons(sport);
    udp->dest   = htons(dport);
    udp->len    = htons(sizeof(struct udphdr) + plen);
    udp->check  = 0;
    memcpy(packet + sizeof(struct udphdr), payload, plen);
    udp->check  = udp_checksum(saddr, daddr, udp, payload, plen);

    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(dport);
    dst.sin_addr.s_addr = daddr;
    sendto(raw, packet, sizeof(struct udphdr) + plen, 0,
           (struct sockaddr *)&dst, sizeof(dst));
}

int main() {
    setvbuf(stdout, NULL, _IOLBF, 0);

    int guard = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ga = {0};
    ga.sin_family = AF_INET; ga.sin_addr.s_addr = htonl(INADDR_ANY); ga.sin_port = htons(SRV_PORT);
    int opt = 1; setsockopt(guard, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(guard, (struct sockaddr *)&ga, sizeof(ga));

    int raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw == ERROR) { perror("socket(SOCK_RAW) — нужен root"); exit(ERROR); }

    printf("Raw echo server on UDP port %d\n", SRV_PORT);

    uint8_t buf[2048];
    while (1) {
        ssize_t n = recv(raw, buf, sizeof(buf), 0);
        if (n == ERROR) { perror("recv"); continue; }

        struct iphdr *ip = (struct iphdr *)buf;
        if (ip->protocol != IPPROTO_UDP) continue;
        int ihl = ip->ihl * 4;
        struct udphdr *udp = (struct udphdr *)(buf + ihl);

        if (udp->dest != htons(SRV_PORT)) continue;

        uint8_t *pl = buf + ihl + sizeof(struct udphdr);
        int pllen = ntohs(udp->len) - sizeof(struct udphdr);
        if (pllen < 1) continue;

        uint8_t op = pl[0];
        char text[MAX_TEXT];
        int tlen = pllen - 1;
        if (tlen > MAX_TEXT - 1) tlen = MAX_TEXT - 1;
        memcpy(text, pl + 1, tlen);
        text[tlen] = '\0';

        uint32_t cip = ip->saddr;
        uint16_t cport = ntohs(udp->source);
        char ipstr[32];
        struct in_addr ina = { cip };
        snprintf(ipstr, sizeof(ipstr), "%s", inet_ntoa(ina));

        if (op == OP_CLOSE) {
            client_t *c = find_client(cip, cport);
            if (c) c->active = 0;
            printf("[close] %s:%d -> счётчик сброшен\n", ipstr, cport);
            continue;
        }

        client_t *c = find_client(cip, cport);
        if (!c) c = add_client(cip, cport);
        if (!c) continue;
        c->count++;
        printf("[%s:%d #%ld] %s\n", ipstr, cport, c->count, text);

        char reply[MAX_TEXT + 32];
        reply[0] = OP_DATA;
        int rl = snprintf(reply + 1, sizeof(reply) - 1, "%s %ld", text, c->count);

        send_reply(raw, ip->daddr, cip, SRV_PORT, cport, reply, rl + 1);
    }

    close(raw);
    close(guard);
    return 0;
}
