

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#define PORT 3001
#define MAX_DATA 100

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15
#endif
#define EPOLL_SIZE 50

void broadcast(int* arr, int fd, char* buffer, int bufferSize);
void init(int* arr);
void deleteFdNum(int* arr, int fd);
void addFdNum(int* arr, int fd);

int main()
{
    int ret = -1;
    int serverSock;
    int acceptedSock;
    char input[10] ="input";
    struct sockaddr_in serv_adr, clnt_adr;
    char readBuf[MAX_DATA];
    socklen_t adr_sz =  sizeof(serv_adr);
    int optval = 1;
    struct epoll_event* ep_events;
    struct epoll_event event;
    int epfd, event_cnt, i, str_len;
    int fdNumArr[10]; 

    init(fdNumArr);
    if ((serverSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto leave;
    }

    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = INADDR_ANY;
    serv_adr.sin_port = htons(PORT);
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    if ((ret = bind(serverSock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)))) {
        perror("bind");
        goto error;
    }

    if ((ret = listen(serverSock, 1))) {
        perror("listen");
        goto error;
    }

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
    event.events = EPOLLIN;
    event.data.fd = serverSock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &event);
    

    
    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1){
            perror("epoll_wait");
            goto error;
        }


        for(i = 0; i < event_cnt; i++){
            if(ep_events[i].data.fd == serverSock){
                adr_sz = sizeof(clnt_adr);
                acceptedSock = accept(serverSock, (struct sockaddr*)&clnt_adr, &adr_sz);
                event.events = EPOLLIN;
                event.data.fd = acceptedSock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, acceptedSock, &event);
                addFdNum(fdNumArr, event.data.fd);
                printf("connected client: %d \n", acceptedSock);
            } else {
                str_len = read(ep_events[i].data.fd, readBuf, MAX_DATA - 1);
                if(str_len == 0){
                    deleteFdNum(fdNumArr, ep_events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    
                    printf("closed client: %d \n", ep_events[i].data.fd);
                }else {
                    readBuf[str_len] = '\0';
                    printf("%s", readBuf);
                    broadcast(fdNumArr, ep_events[i].data.fd, readBuf, str_len);
                }
            }
        }
    }

    close(serverSock);
    close(epfd);
    return 0;
	
error:
    close(serverSock);
leave:
    return ret;
}

void addFdNum(int* arr, int fd){
    int i;
    for(i = 0; i < 10; i++){
        if(arr[i] == 0){
            arr[i] = fd;
            break;
        }
    }
}

void deleteFdNum(int* arr, int fd){
    int i;
    for(i = 0; i < 10; i++){
        if(arr[i] == fd)
            arr[i] = 0;    
    }
}

void init(int* arr){
    int i = 0;
    for(i = 0; i< 10; i++)
        arr[i] = 0;
    
}

void broadcast(int* arr, int fd, char* buffer, int bufferSize){
    //write(fd, readBuf, str_len);
    int i;
    for(i = 0; i < 10; i++){
        if(arr[i]== fd || arr[i] == 0)
            continue;
        write(arr[i], buffer, bufferSize);
    }
}