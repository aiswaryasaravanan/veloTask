#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

int main(){

    char msg[256] = "Hello from server";
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);

    int clientSocket;
    clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);

    send(clientSocket, msg, sizeof(msg), 0);

    return 0;
}