//
// client.c
// simple_client
//
// Created by Injae Lee on 2014. 10. 10.
// Modified by Minsuk Lee,
// Copyright (c) 2014. Injae Lee All rights reserved.
// see LICENSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PORT 3002
#define WRITE_DATA "Hello dayoungle!"
#define MAX_DATA 100

int main()
{
    int ret = -1;//이게 뭐하는 인자인지.
    int clientSock;
    char input[10] ="input";
    struct sockaddr_in serverAddr;
    char buffer[MAX_DATA];
    if ((clientSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");//perror도 뭐하는 녀석인지.이 밑줄도 이런 문법은 뭐지?
        goto leave;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    if ((ret = connect(clientSock,(struct sockaddr*)&serverAddr,
                       sizeof(serverAddr)))) {
        perror("connect");
        goto error;
    }
    while(1){
        printf("%s\n", input);
        fgets(buffer, MAX_DATA,stdin);
        //종료 조건
        if(!strcmp(buffer, "q\n") || !strcmp(buffer, "Q\n"))
            break;
        if ((ret = send(clientSock, buffer, sizeof(buffer), 0)) <= 0) {
            perror("send");
            ret = -1;
        } else
            printf("I Client:  %s \n", buffer);
        
        if ((ret = recv(clientSock, buffer, MAX_DATA, 0)) <= 0) {
            perror("recv");
            ret = -1;
        } else
            printf("You Server: %s \n",  buffer);
    } 

error:
    close(clientSock);
leave:
    return ret;
}

