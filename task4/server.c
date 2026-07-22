#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUF_SIZE 512

typedef struct {
    char username[20];
    char password[20];
} User;

User users[2] = {{"alice", "alice123"}, {"bob", "bob123"}};

int authenticate(char *u, char *p) {
    for (int i = 0; i < 2; i++) {
        if (!strcmp(u, users[i].username) && !strcmp(p, users[i].password)) {
            return 1;
        }
    }
    return 0;
}

void *handleClient(void *arg) {
    int sock = *(int *)arg;
    free(arg);
    char buf[BUF_SIZE], cmd[20], u[20], p[20];
    int authed = 0;

    send(sock, "200 CONNECTED ST5004CEM Server\n", 31, 0);

    while (recv(sock, buf, BUF_SIZE - 1, 0) > 0) {
        buf[strcspn(buf, "\r\n")] = 0;
        sscanf(buf, "%19s %19s %19s", cmd, u, p);

        if (!strcmp(cmd, "LOGIN")) {
            if (authenticate(u, p)) {
                authed = 1;
                send(sock, "200 SUCCESS Welcome user\n", 25, 0);
            } else {
                send(sock, "401 AUTH_FAILED\n", 16, 0);
            }
        } else if (!strcmp(cmd, "EXIT")) {
            send(sock, "200 BYE\n", 8, 0);
            break;
        } else if (!authed) {
            send(sock, "403 DENIED Please LOGIN\n", 24, 0);
        } else if (!strcmp(cmd, "CALC")) {
            int a = atoi(u), b = atoi(p);
            snprintf(buf, sizeof(buf), "200 RESULT: %d\n", a + b);
            send(sock, buf, strlen(buf), 0);
        } else {
            snprintf(buf, sizeof(buf), "200 ECHO: %s\n", buf);
            send(sock, buf, strlen(buf), 0);
        }
        memset(buf, 0, BUF_SIZE);
    }
    close(sock);
    return NULL;
}

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0), opt = 1;
    struct sockaddr_in saddr = {.sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY};

    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(sfd, 5);

    printf("=== Multi-Threaded Socket Server Running on Port %d ===\n", PORT);

    while (1) {
        int *csock = malloc(sizeof(int));
        *csock = accept(sfd, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, handleClient, csock);
        pthread_detach(tid);
    }
    close(sfd);
    return 0;
}