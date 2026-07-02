#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>

#define BUFFER_LENGTH 128

struct conn_item {
    int fd;
    char buffer[BUFFER_LENGTH];
    int idx;
};

struct conn_item connlist[1024] = {0};

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

    int epfd = epoll_create(1);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    struct epoll_event events[1024] = {0};
    while (1) {
        int nready = epoll_wait(epfd, events, 1024, -1);

        int i = 0;
        for (i = 0; i < nready; i++) {
            int connfd = events[i].data.fd;
            if (connfd == sockfd) {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
                
                printf("socket: %d\n", clientfd);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

                connlist[clientfd].fd = clientfd;
                memset(connlist[clientfd].buffer, 0, BUFFER_LENGTH);
                connlist[clientfd].idx = 0;

            } else if (events[i].events & EPOLLIN) {
                char *buffer = connlist[connfd].buffer;
                int idx = connlist[connfd].idx;
                int count = recv(connfd, buffer, BUFFER_LENGTH - idx, 0);
                if (count == 0) {
                    printf("disconnect\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
                    close(connfd);
                    continue;
                }
                connlist[connfd].idx += count;

                send(connfd, buffer, count, 0);
                printf("clientfd: %d, count: %d, buffer: %s\n", connfd, count, buffer);
            }
        }
    }

    getchar();
}
