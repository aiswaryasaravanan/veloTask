#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>         //INADDR_ANY

int acceptSocket(int sock,int port){
    sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;             //ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;     
    serverAddress.sin_port = htons(port);           //host to network byte addr translation

    int bindStatus = bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(bindStatus < 0)
        printf("%d", bindStatus);

    int listenStatus = listen(sock, 5);
    if(listenStatus < 0)                            //backlog queue size=5(maximum no. of connection it can wait for)
        printf("%d", listenStatus);

    int sockNew = accept(sock, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);
    if(sockNew < 0){
        printf("%d",sockNew);
        exit(0);
    }return sockNew;
}

int connectSocket(int sock, int port){
    sock  = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress1;
    serverAddress1.sin_family = AF_INET;
    serverAddress1.sin_port = htons(port);
    serverAddress1.sin_addr.s_addr = INADDR_ANY;

    int connectStatus = connect(sock, (struct sockaddr*)&serverAddress1, sizeof(serverAddress1));
    if(connectStatus < 0){
        printf("%d\n", connectStatus);
        exit(0);
    }
    return sock;
}