#ifndef RAW_UTIL_H
#define RAW_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

static inline uint16_t checksum16(const void *data, size_t len) {
    const uint16_t *p = data;
    uint32_t sum = 0;
    while (len > 1) { sum += *p++; len -= 2; }
    if (len == 1) sum += *(const uint8_t *)p;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}

struct pseudo_udp {
    uint32_t saddr;
    uint32_t daddr;
    uint8_t  zero;
    uint8_t  proto;
    uint16_t udp_len;
};

static inline uint16_t udp_checksum(uint32_t saddr, uint32_t daddr,
                                    struct udphdr *udp, const void *payload, size_t plen) {
    size_t udplen = sizeof(struct udphdr) + plen;
    size_t total  = sizeof(struct pseudo_udp) + udplen;
    uint8_t buf[2048];
    if (total > sizeof(buf)) return 0;

    struct pseudo_udp ph;
    ph.saddr = saddr;
    ph.daddr = daddr;
    ph.zero  = 0;
    ph.proto = IPPROTO_UDP;
    ph.udp_len = htons((uint16_t)udplen);

    memcpy(buf, &ph, sizeof(ph));
    memcpy(buf + sizeof(ph), udp, sizeof(struct udphdr));
    memcpy(buf + sizeof(ph) + sizeof(struct udphdr), payload, plen);

    return checksum16(buf, total);
}

#endif
