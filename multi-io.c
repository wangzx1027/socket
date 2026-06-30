#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

// block
void *client_thread(void *arg) {
    int clientfd = *(int *)arg;
    while (1) {
        char buffer[128] = {0};
        int count = recv(clientfd, buffer, sizeof(buffer), 0);
        if (count == 0) {
            break;
        }
        send(clientfd, buffer, count, 0);
        printf("clientfd: %d, count: %d, buffer: %s\n", clientfd, count, buffer);
    }
    close(clientfd);
}


int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(2048);

    if (-1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr))) {
        perror("bind");
        return 1;
    }

    listen(sockfd, 10);

#if 0
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
    printf("accept a client\n");
#if 0
    char buffer[128] = {0};
    int count = recv(clientfd, buffer, sizeof(buffer), 0);
    send(clientfd, buffer, count, 0);
    printf("sockfd:%d, clientfd: %d, count: %d, buffer: %s\n", sockfd, clientfd, count, buffer);
#else
    while (1) {
        char buffer[128] = {0};
        int count = recv(clientfd, buffer, sizeof(buffer), 0);
        if (count == 0) {
            break;
        }
        send(clientfd, buffer, count, 0);
        printf("sockfd:%d, clientfd: %d, count: %d, buffer: %s\n", sockfd, clientfd, count, buffer);
    }
#endif

#elif 0
    while (1) {
        struct sockaddr_in clientaddr;
        socklen_t len = sizeof(clientaddr);
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

        pthread_t thid;
        pthread_create(&thid, NULL, client_thread, &clientfd);
    }
#else // select
    // int nready = select(maxfd, rset, wset, eset, timeout);

    fd_set rfds, rset;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    int maxfd = sockfd;

    while (1) {
        rset = rfds;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);
            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            
            printf("socket: %d\n", clientfd);

            FD_SET(clientfd, &rfds);
            maxfd = clientfd;
        }

        int i = 0;
        for (i = sockfd + 1; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {
                char buffer[128] = {0};
                int count = recv(i, buffer, sizeof(buffer), 0);
                if (count == 0) {
                    printf("disconnect\n");
                    FD_CLR(i, &rfds);
                    close(i);
                    break;
                }
                send(i, buffer, count, 0);
                printf("clientfd: %d, count: %d, buffer: %s\n", i, count, buffer);
            }
        }
    }

#endif
    getchar();
}
