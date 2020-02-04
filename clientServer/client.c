//Sender who fragments the packet if the packet size is greater than MTU

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>    
#include <string.h>  
#include <math.h>
#include <time.h>        //for srand function

#define MTU 30
#define VERSION 4
#define HEADERLENGTH 20

typedef struct{
    int DF;
    int MF;
}IPFlag;

typedef struct{
    int version; 
    int headerLength;
    int totalLength;
    IPFlag ipflag;
    int fragmentOffset;
    // char* sourceAddress;
    // char* destinationAddress;
    char data[200];
}Packet;

int initClient(int);
Packet fragmentPacket(Packet, int, int);
int isYetToSend(int*, int);
void updateCheckList(int*, int, int);
Packet setHeader(Packet, int, int);
Packet setFlag (Packet);
void shuffleAndSend(int, Packet*, int);

//connect with server socket
int initClient(int clientSocket){
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    return connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
}

//cutting down the packet data into fragments as per MTU value using strncpy
Packet fragmentPacket(Packet packet, int fragOffset, int startIndex){
    Packet fragmentedPacket;
    fragmentedPacket = setHeader(fragmentedPacket, fragOffset, packet.totalLength-packet.headerLength);
    strncpy(fragmentedPacket.data, packet.data+startIndex, (MTU - packet.headerLength));
    fragmentedPacket = setFlag(fragmentedPacket);
    return fragmentedPacket;
}

//check if all the fragments would have been sent
int isYetToSend(int checkList[], int noOfFragments){
    for(int i=0; i<noOfFragments; i++)
        if(checkList[i] != -1)
            return 1;  
    return 0;
}

//keep track of whatever fragment sent
void updateCheckList(int checkList[], int randPacket, int noOfFragments){
    for(int i=0; i<noOfFragments; i++)
        if(checkList[i] == randPacket){
            checkList[i] = -1;
            break;
        }
}

void printFragment(Packet fragment){
    printf("version:%d\t", fragment.version);
    printf("fragmentOffset:%d\t", fragment.fragmentOffset);
    printf("headerLength:%d\n", fragment.headerLength);
    printf("Data:%s\t", fragment.data);
    printf("totalLength:%d\n", fragment.totalLength);
    printf("DF:%d\t", fragment.ipflag.DF);
    printf("MF:%d\n\n\n", fragment.ipflag.MF);
}

//generate a random number and send the fragments accordingly
void shuffleAndSend(int noOfFragments, Packet *fragmentedPackets, int clientSocket){
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
        printFragment(fragmentedPackets[randPacket]);
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


Packet setHeader(Packet packet, int fragmentOffset, int dataLength){
    packet.version = VERSION;
    packet.headerLength = HEADERLENGTH;
    // packet.data[200] = data;
    // packet.totalLength = strlen(packet.data) + packet.headerLength;
    packet.totalLength = dataLength + HEADERLENGTH;
    packet.fragmentOffset = fragmentOffset;
    return packet;
}

Packet setFlag(Packet fragmentedPacket){
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

    Packet packet;
    Packet fragment;

    int clientSocket  = socket(AF_INET, SOCK_STREAM, 0);
    
    int connectStatus = initClient(clientSocket);
    if(connectStatus < 0)
        printf("%d\n", connectStatus);

    scanf("%s",packet.data);
    packet = setHeader(packet, 0, strlen(packet.data)); 

    if(packet.totalLength > MTU){
        int noOfFragments = (int)ceil((float)(packet.totalLength - packet.headerLength)/(float)(MTU - packet.headerLength));
        Packet fragmentedPackets[noOfFragments];
        int fragOffset = 0;
        int index = 0;

        for(int i = 0; i < (packet.totalLength - packet.headerLength); i += (MTU - packet.headerLength))
            fragmentedPackets[index++] = fragmentPacket(packet, fragOffset++, i);
        shuffleAndSend(noOfFragments, fragmentedPackets, clientSocket);
    }
    else
        send(clientSocket, (void *)&packet, sizeof(packet), 0);

    close(clientSocket);
    return 0;
}
