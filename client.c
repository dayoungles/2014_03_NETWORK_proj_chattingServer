
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define IP "127.0.0.1"
#define PORT 3001
#define MAX_DATA 100
#define EPOLL_SIZE 50

int main()
{
    int ret = -1;
    int clientSock;
    char input[10] ="input";
    struct sockaddr_in serverAddr;
    char buffer[MAX_DATA];
    socklen_t adr_sz =  sizeof(serverAddr);
    int optval = 1;
    struct epoll_event* ep_events;
    struct epoll_event event;
    int epfd, event_cnt, i, str_len;

    if ((clientSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto leave;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    if ((ret = connect(clientSock,(struct sockaddr*)&serverAddr, sizeof(serverAddr)))) {
        perror("connect");
        goto error;
    }

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    event.events = EPOLLIN;
    event.data.fd = clientSock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &event);
    event.data.fd = 0;
    epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);

    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1){
            perror("epoll_wait");
            goto error;
        }
        
        for(i=0; i < event_cnt; i++){
            if(ep_events[i].data.fd == 0){
                fgets(buffer, MAX_DATA,stdin);
                if ((ret = send(clientSock, buffer, strlen(buffer), 0)) <= 0) {
                    perror("send");
                     ret = -1;
                 } else
                    printf("I :  %s \n", buffer);
            } else {
                if ((ret = recv(clientSock, buffer, MAX_DATA, 0)) <= 0) {
                    perror("recv");
                    ret = -1;
                    goto leave;
                } else
                    printf("%d: %s \n",  ep_events[i].data.fd,buffer);
            }
        }

    }
error:
    close(clientSock);
leave:
    return ret;
}

