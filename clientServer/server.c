#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>

int connectSocket(int);
// void printFragmentedPackets(struct Packet);

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
    char data[200];
};


int connectSocket(int serverSocket){
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;             //ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;     
    serverAddress.sin_port = htons(8080);           //host to network byte addr translation

    int bindStatus = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(bindStatus < 0)
        printf("%d", bindStatus);

    int listenStatus = listen(serverSocket, 5);
    if(listenStatus < 0)                            //backlog queue size=5(maximum no. of connection it can wait for)
        printf("%d", listenStatus);

    return accept(serverSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);
}


void printFragmentedPackets(struct Packet packet){
    printf("Version:%d\n", packet.version);
    printf("HeaderLength:%d\n", packet.headerLength);
    printf("TotalLength:%d\n", packet.totalLength);
    printf("FragmentOffset:%d\n", packet.fragmentOffset);
    printf("Data:%s\n",packet.data);
    printf("DF flag:%d\n", packet.ipflag.DF);
    printf("MF flag:%d\n\n\n\n", packet.ipflag.MF);
}

int main(){

    struct Packet packet;
    // packet.data = (char*)calloc(200,sizeof(char));

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    int clientSocket = connectSocket(serverSocket);
    if(clientSocket < 0)
        printf("%d",clientSocket);

    int readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);

    while(readStatus){
        printFragmentedPackets(packet);
        readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    }

    close(clientSocket);
    return 0;

}

