#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

int main(){

    char msg[256] = "Hello from server";
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0); //socket file descriptor

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;             //ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;     //any available protocol
    serverAddress.sin_port = htons(8080);           //host to network byte addr translation

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);                        //backlog queue size=5(maximum no. of connection it can wait for)

    int clientSocket;
    clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);

    send(clientSocket, msg, sizeof(msg), 0);

    return 0;

}