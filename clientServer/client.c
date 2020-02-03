#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>    
#include<string.h>  
#include<math.h>
#include<time.h>        //for srand function

int connectSocket(int);
struct Packet fragmentPacket(struct Packet, int, int, int);
int isYetToSend(int*, int);
void updateCheckList(int*, int, int);
struct Packet setHeader(struct Packet, int, int, int, int);
struct Packet setFlag(struct Packet, int);
void shuffleAndSend(int, struct Packet*, int);

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

int connectSocket(int clientSocket){
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    return connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
}

struct Packet fragmentPacket(struct Packet packet, int fragOffset, int MTU, int startIndex){
    struct Packet fragmentedPacket;
    fragmentedPacket = setHeader(fragmentedPacket, packet.version, packet.headerLength, fragOffset++, packet.totalLength-packet.headerLength);
    strncpy(fragmentedPacket.data, packet.data+startIndex, (MTU - packet.headerLength));
    fragmentedPacket = setFlag(fragmentedPacket, MTU);
    return fragmentedPacket;
}

int isYetToSend(int checkList[], int noOfFragments){
    for(int i=0; i<noOfFragments; i++)
        if(checkList[i] != -1)
            return 1;  
    return 0;
}

void updateCheckList(int checkList[], int randPacket, int noOfFragments){
    for(int i=0; i<noOfFragments; i++)
        if(checkList[i] == randPacket){
            checkList[i] = -1;
            break;
        }
}

void shuffleAndSend(int noOfFragments, struct Packet *fragmentedPackets, int clientSocket){
    srand(time(NULL));
    int randPacket;
    int checkList[noOfFragments];
    for(int i=0; i<noOfFragments; i++)
        checkList[i] = i;
    while(isYetToSend(checkList, noOfFragments)){
        randPacket = rand() % noOfFragments;
        updateCheckList(checkList, randPacket, noOfFragments);
        printf("%d\n",randPacket);
        send(clientSocket, (void *)&fragmentedPackets[randPacket], sizeof(fragmentedPackets[randPacket]), 0);
    }
}

//To drop some packet..

// void shuffleAndSend(int noOfFragments, struct Packet *fragmentedPackets, int clientSocket){
//     srand(time(NULL));
//     int index;
//     for(int j=0; j<noOfFragments; j++){
//         index = rand() % noOfFragments;
//         printf("%d\n",index);
//         send(clientSocket, (void *)&fragmentedPackets[index], sizeof(fragmentedPackets[index]), 0);
//     }
// }


struct Packet setHeader(struct Packet packet, int version, int headerLength, int fragmentOffset, int dataLength){
    packet.version = version;
    packet.headerLength = headerLength;
    // packet.data[200] = data;
    // packet.totalLength = strlen(packet.data) + packet.headerLength;
    packet.totalLength = dataLength + packet.headerLength;
    packet.fragmentOffset = fragmentOffset;
    return packet;
}

struct Packet setFlag(struct Packet fragmentedPacket, int MTU){
    if((fragmentedPacket.fragmentOffset+1)* (MTU - fragmentedPacket.headerLength) < fragmentedPacket.totalLength - fragmentedPacket.headerLength){
        fragmentedPacket.ipflag.MF = 1;
        fragmentedPacket.ipflag.DF = 0;
    }
    else{
        fragmentedPacket.ipflag.DF = 1;
        fragmentedPacket.ipflag.MF = 0;
    }
    return fragmentedPacket;
}

int main(){

    struct Packet packet;
    struct Packet fragment;
    int MTU = 30;     

    scanf("%s",packet.data);
    packet = setHeader(packet, 4, 20, 0, strlen(packet.data)); 

    int clientSocket  = socket(AF_INET, SOCK_STREAM, 0);

    int connectStatus = connectSocket(clientSocket);
    if(connectStatus < 0)
        printf("%d\n", connectStatus);

    if(packet.totalLength > MTU){
        int noOfFragments = (int)ceil((float)(packet.totalLength - packet.headerLength)/(float)(MTU - packet.headerLength));
        struct Packet fragmentedPackets[noOfFragments];
        int fragOffset = 0;
        int index = 0;

        for(int i = 0; i < (packet.totalLength - packet.headerLength); i += (MTU - packet.headerLength))
            fragmentedPackets[index++] = fragmentPacket(packet, fragOffset++, MTU, i);
        shuffleAndSend(noOfFragments, fragmentedPackets, clientSocket);
    }
    else
        send(clientSocket, (void *)&packet, sizeof(packet), 0);

    close(clientSocket);
    return 0;
}

