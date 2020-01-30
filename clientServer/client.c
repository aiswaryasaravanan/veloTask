#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>    
#include<string.h>  
#include<math.h>
#include<time.h>        //for srand function

// char* getData();
int connectSocket(int);
struct Packet setHeader();
struct Packet setFlag(struct Packet, int);


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

// char* getData(){
//     char data[200];
//     // data = (char*)calloc(200,sizeof(char));
//     scanf("%s",data);
//     return data;
// }

struct Packet setHeader(struct Packet packet, int version, int headerLength, int fragmentOffset){
    packet.version = 4;
    packet.headerLength = 20;
    // packet.data[200] = data;
    packet.totalLength = strlen(packet.data) + packet.headerLength;
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
    int MTU = 30;     

    // packet.data = getData()
    scanf("%s",packet.data);
    packet = setHeader(packet, 4, 20, 0);

    int clientSocket  = socket(AF_INET, SOCK_STREAM, 0);

    int connectStatus = connectSocket(clientSocket);
    if(connectStatus < 0)
        printf("%d\n", connectStatus);

    if(packet.totalLength > MTU){

        int noOfFragments = (int)ceil((float)(packet.totalLength - packet.headerLength)/(float)(MTU - packet.headerLength));
        struct Packet fragmentedPacket,fragmentedPackets[noOfFragments];
        int fragOffset = 0;
        int index = 0;

        for(int i = 0; i < (packet.totalLength - packet.headerLength); i += (MTU - packet.headerLength)){
            fragmentedPacket = setHeader(fragmentedPacket, packet.version, packet.headerLength, fragOffset++);
            strncpy(fragmentedPacket.data, packet.data+i, (MTU - packet.headerLength));
            fragmentedPacket = setFlag(fragmentedPacket, MTU);

            fragmentedPackets[index++] = fragmentedPacket;
        }

        //shuffling Fragments
        srand(time(NULL));
        for(int j=0; j<noOfFragments; j++){
            index = rand() % noOfFragments;
            send(clientSocket, (void *)&fragmentedPackets[index], sizeof(fragmentedPackets[index]), 0);
        }
    }
    else{
        send(clientSocket, (void *)&packet, sizeof(packet), 0);
    }

    close(clientSocket);

    return 0;
}

