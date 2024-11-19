#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char *argv[]) {
    // Проверка на наличие достаточного количества аргументов
    if (argc < 3) {
        printf("Usage: %s <SERV_PORT> <BUFSIZE>\n", argv[0]);
        exit(1);
    }

    // Получение SERV_PORT и BUFSIZE из аргументов командной строки
    int SERV_PORT = atoi(argv[1]);
    int BUFSIZE = atoi(argv[2]);

    if (SERV_PORT <= 0 || BUFSIZE <= 0) {
        printf("Invalid port or buffer size\n");
        exit(1);
    }

    int sockfd, n;
    char mesg[BUFSIZE], ipadr[16];
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        exit(1);
    }

    memset(&servaddr, 0, SLEN);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(sockfd, (SADDR *)&servaddr, SLEN) < 0) {
        perror("bind problem");
        exit(1);
    }
    printf("SERVER starts on port %d...\n", SERV_PORT);

    while (1) {
        unsigned int len = SLEN;

        if ((n = recvfrom(sockfd, mesg, BUFSIZE, 0, (SADDR *)&cliaddr, &len)) < 0) {
            perror("recvfrom");
            exit(1);
        }
        mesg[n] = 0;

        printf("REQUEST %s FROM %s : %d\n", mesg,
               inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
               ntohs(cliaddr.sin_port));

        if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
            perror("sendto");
            exit(1);
        }
    }
}
