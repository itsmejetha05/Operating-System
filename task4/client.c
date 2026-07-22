#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 512

void printUsage() {
    printf("--- Client Control Protocol ---\n");
    printf("1. LOGIN <user> <pass>\n");
    printf("2. CALC <num1> <num2>\n");
    printf("3. <any text message>\n");
    printf("4. EXIT\n");
    printf("-------------------------------\n");
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in saddr = {.sin_family = AF_INET, .sin_port = htons(PORT)};

    if (inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr) <= 0) {
        printf("Invalid Address\n");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        printf("Connection Failed! Start server first.\n");
        return 1;
    }

    char buf[BUF_SIZE];
    int len = recv(sock, buf, BUF_SIZE - 1, 0);
    if (len > 0) {
        buf[len] = '\0';
        printf("%s", buf);
    }

    printUsage();

    while (1) {
        printf("\nclient> ");
        if (!fgets(buf, BUF_SIZE, stdin)) break;

        send(sock, buf, strlen(buf), 0);

        if (!strncmp(buf, "EXIT", 4)) {
            memset(buf, 0, BUF_SIZE);
            recv(sock, buf, BUF_SIZE - 1, 0);
            printf("server> %s", buf);
            break;
        }

        memset(buf, 0, BUF_SIZE);
        len = recv(sock, buf, BUF_SIZE - 1, 0);
        if (len <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        buf[len] = '\0';
        printf("server> %s", buf);
    }

    close(sock);
    return 0;
}