#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

int connectSocket(int);


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

struct Packet setHeader(struct Packet packet, char *data){
    struct Packet defragmentedPacket;
    defragmentedPacket.version = packet.version;
    defragmentedPacket.headerLength = packet.headerLength;
    defragmentedPacket.totalLength = strlen(data) + packet.headerLength;
    defragmentedPacket.fragmentOffset = 0;
    defragmentedPacket.ipflag.DF = 0;
    defragmentedPacket.ipflag.MF = 0;
    strcpy(defragmentedPacket.data,data);
    return defragmentedPacket;
}

void printDefragmentedPacket(struct Packet defragmentedPacket){
    printf("version:%d\n", defragmentedPacket.version);
    printf("headerLength:%d\n", defragmentedPacket.headerLength);
    printf("DF:%d\n", defragmentedPacket.ipflag.DF);
    printf("MF:%d\n", defragmentedPacket.ipflag.MF);
    printf("fragmentOffset:%d\n", defragmentedPacket.fragmentOffset);
    printf("Data:%s\n", defragmentedPacket.data);
    printf("totalLength:%d\n", defragmentedPacket.totalLength);
}

int main(){

    struct Packet packet;
    struct Packet defragmentedPacket;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int clientSocket = connectSocket(serverSocket);
    if(clientSocket < 0)
        printf("%d",clientSocket);

    int readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    int packetCount = 0;

    // Drops, if out of order

    int flag;
    while(readStatus){
        if(packetCount == packet.fragmentOffset){
            strcat(defragmentedPacket.data, packet.data);
            packetCount++;
            flag = 1;
        }else{
            printf("Packets out of order :)\n");
            flag = 0;
            break;
        }
        readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    }

    if(flag == 1){
        defragmentedPacket = setHeader(packet, defragmentedPacket.data);
        printDefragmentedPacket(defragmentedPacket);
    }

    close(clientSocket);
    return 0;

}

