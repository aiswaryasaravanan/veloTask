#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>

int main(){
    int clientSocket ;
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int connectStatus = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(connectStatus < 0)
        printf("Error connectiong the socket");

    char buffer[256] = "";
    int readStatus = read(clientSocket, buffer, sizeof(buffer));
    printf("%s", buffer);

    return 0;
}