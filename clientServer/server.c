#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>

struct IPFlag{
    int DF;
    int MF;
};

struct Packet{
    int version; 
    int headerLength;
    int totalLength;
    struct IPFlag ipflag;
    int fragmentOffset;
    // char* sourceAddress;
    // char* destinationAddress;
    char *data;
};

int main(){

    struct Packet packet;

    int serverSocket;    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;             //ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;     //any available protocol
    serverAddress.sin_port = htons(8080);           //host to network byte addr translation

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);                        //backlog queue size=5(maximum no. of connection it can wait for)

    int clientSocket;
    clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);

    int readStatus = read(clientSocket, (struct Packet*)&packet, sizeof(packet));

    printf("Version:%d\n", packet.version);
    printf("HeaderLength:%d\n", packet.headerLength);
    printf("TotalLength:%d\n", packet.totalLength);
    printf("FragmentOffset:%d\n", packet.fragmentOffset);
    printf("DF flag:%d\n", packet.ipflag.DF);
    printf("MF flag:%d\n\n\n\n", packet.ipflag.MF);

    // while(packet.ipflag.MF == 1){
    //     readStatus = read(clientSocket, (struct Packet*)&packet, sizeof(packet));
    //     printf("Version:%d\n", packet.version);
    //     printf("HeaderLength:%d\n", packet.headerLength);
    //     printf("TotalLength:%d\n", packet.totalLength);
    //     printf("FragmentOffset:%d\n", packet.fragmentOffset);
    //     printf("DF flag:%d\n", packet.ipflag.DF);
    //     printf("MF flag:%d\n\n\n\n", packet.ipflag.MF);
    // }

    close(clientSocket);
    
    return 0;

}