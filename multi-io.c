
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


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
    
    getchar();
    close(clientfd);
}