#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <sys/time.h>

#include "raw_util.h"

#define SRV_IP   "127.0.0.1"
#define SRV_PORT 7100
#define CLI_PORT 33333
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *msg = argc > 1 ? argv[1] : "hello from raw eth+ip+udp";
    const char *ifname = argc > 2 ? argv[2] : "lo";
    size_t mlen = strlen(msg);

    uint32_t saddr = inet_addr(SRV_IP);
    uint32_t daddr = inet_addr(SRV_IP);

    int ifindex = if_nametoindex(ifname);
    if (ifindex == 0) { perror("if_nametoindex"); exit(ERROR); }

    int guard = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ga = {0};
    ga.sin_family = AF_INET; ga.sin_addr.s_addr = htonl(INADDR_ANY); ga.sin_port = htons(CLI_PORT);
    bind(guard, (struct sockaddr *)&ga, sizeof(ga));

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sock == ERROR) { perror("socket(AF_PACKET) — нужен root"); exit(ERROR); }

    int raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (raw == ERROR) { perror("socket(SOCK_RAW)"); exit(ERROR); }
    struct timeval tv = { 3, 0 };
    setsockopt(raw, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t frame[2048];
    struct ether_header *eth = (struct ether_header *)frame;
    struct iphdr  *ip  = (struct iphdr *)(frame + sizeof(struct ether_header));
    struct udphdr *udp = (struct udphdr *)(frame + sizeof(struct ether_header) + sizeof(struct iphdr));

    memset(eth->ether_dhost, 0x00, 6);
    memset(eth->ether_shost, 0x00, 6);
    eth->ether_type = htons(ETH_P_IP);

    size_t udplen = sizeof(struct udphdr) + mlen;
    size_t iptot  = sizeof(struct iphdr) + udplen;

    memset(ip, 0, sizeof(struct iphdr));
    ip->version  = 4;
    ip->ihl      = 5;
    ip->tot_len  = htons(iptot);
    ip->id       = htons(0x5678);
    ip->ttl      = 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr    = saddr;
    ip->daddr    = daddr;
    ip->check    = checksum16(ip, sizeof(struct iphdr));

    udp->source = htons(CLI_PORT);
    udp->dest   = htons(SRV_PORT);
    udp->len    = htons(udplen);
    udp->check  = 0;
    memcpy(frame + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr), msg, mlen);
    udp->check  = udp_checksum(saddr, daddr, udp, msg, mlen);

    size_t framelen = sizeof(struct ether_header) + iptot;

    struct sockaddr_ll sll = {0};
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = ifindex;
    sll.sll_halen    = 6;

    if (sendto(sock, frame, framelen, 0, (struct sockaddr *)&sll, sizeof(sll)) == ERROR) {
        perror("sendto"); exit(ERROR);
    }
    printf("Sent (Ethernet + IP + UDP built by client) via %s: '%s'\n", ifname, msg);

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

    close(sock);
    close(raw);
    close(guard);
    return 0;
}
