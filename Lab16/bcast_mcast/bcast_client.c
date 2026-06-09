#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 6000
#define ERROR -1

int main(int argc, char *argv[]) {
    const char *msg = argc > 1 ? argv[1] : "broadcast hello";

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == ERROR) { perror("socket"); exit(ERROR); }

    int yes = 1;
    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) == ERROR) {
        perror("SO_BROADCAST"); exit(ERROR);
    }

    struct sockaddr_in bc = {0};
    bc.sin_family = AF_INET;
    bc.sin_port = htons(PORT);
    bc.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    if (sendto(s, msg, strlen(msg), 0, (struct sockaddr *)&bc, sizeof(bc)) == ERROR) {
        perror("sendto"); exit(ERROR);
    }
    printf("Broadcasted: %s\n", msg);

    close(s);
    return 0;
}
