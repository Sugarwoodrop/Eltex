#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define GROUP "239.0.0.1"
#define PORT  6001
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *msg  = argc > 1 ? argv[1] : "multicast hello";
    const char *ifip = argc > 2 ? argv[2] : NULL;

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    if (ifip) {
        struct in_addr ia;
        ia.s_addr = inet_addr(ifip);
        setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &ia, sizeof(ia));
    }

    unsigned char ttl = 1;
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    unsigned char loop = 1;
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    struct sockaddr_in grp = {0};
    grp.sin_family = AF_INET;
    grp.sin_port = htons(PORT);
    grp.sin_addr.s_addr = inet_addr(GROUP);

    if (sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&grp, sizeof(grp)) == ERROR) {
        perror("sendto"); exit(ERROR);
    }
    printf("Sent to group %s: %s\n", GROUP, msg);

    close(s);
    return 0;
}
