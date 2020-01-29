#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>    
#include<string.h>     

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

    packet.version = 4;
    packet.headerLength = 20;
    packet.data = (char*)calloc(20,sizeof(char));
    scanf("%s",packet.data);
    packet.totalLength = strlen(packet.data) + packet.headerLength;

    int MTU = 30; 

    int clientSocket ;  
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int connectStatus = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(connectStatus < 0)
        printf("Error connectiong the socket");

    if(packet.totalLength > MTU){
        struct Packet fragmentedPacket;
        int fragOffset = 0;
        for(int i = 0; i <packet.totalLength; i += (MTU - packet.headerLength)){
            fragmentedPacket.version = packet.version;
            fragmentedPacket.headerLength = packet.headerLength;
            fragmentedPacket.totalLength = packet.totalLength;
            fragmentedPacket.fragmentOffset = fragOffset++;
            strncpy(fragmentedPacket.data, packet.data+i, (MTU - packet.headerLength));
            if((fragmentedPacket.fragmentOffset+1)* (MTU - packet.headerLength) < fragmentedPacket.totalLength - fragmentedPacket.headerLength){
                fragmentedPacket.ipflag.MF = 1;
                fragmentedPacket.ipflag.DF = 0;
            }
            else{
                fragmentedPacket.ipflag.DF = 1;
                fragmentedPacket.ipflag.MF = 0;
            }


            if(fragOffset >10)
                break;

            send(clientSocket, (void *)&fragmentedPacket, sizeof(fragmentedPacket), 0);
        }
    }else{
        send(clientSocket, (void *)&packet, sizeof(packet), 0);
    }

    return 0;
}