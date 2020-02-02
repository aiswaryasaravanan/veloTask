#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

int connectSocket(int);

int rear = -1;
int front = 0;
int expectedFragment = 0;
int processedFragmentCount = -1;

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
    // return accept(serverSocket, NULL, NULL);

}

struct Packet setHeader(struct Packet fragment, char *data){
    struct Packet defragmentedPacket;
    defragmentedPacket.version = fragment.version;
    defragmentedPacket.headerLength = fragment.headerLength;
    defragmentedPacket.totalLength = fragment.totalLength;
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

// int isOrder(struct Packet packet, struct Packet defragmentedPacket, int packetCount){
//     if(packetCount == packet.fragmentOffset){
//         strcat(defragmentedPacket.data, packet.data);
//         packetCount++;
//         return 1;
//     }
//     return 0;
// }

// struct Packet* shiftQueue(struct Packet bufferQueue[], int index){

//     return bufferQueue;
// }

// int isInQueue(struct Packet bufferQueue[], int packetCount){
//     int i;
//     for(i=0;i<5;i++)
//         if(bufferQueue[i].fragmentOffset == packetCount){
//             bufferQueue = shiftQueue(bufferQueue,i);
//             break;
//         }
//     if(i==5)
//         return -1;
//     else
//         return i;
// }

// int isOrder(int packetCount, struct Packet packet){
//     if(packetCount == packet.fragmentOffset){
//         return 1;
//     }return 0;
// }

// void printQ(struct Packet* bufferQueue){
//     for(int i=0;i<5;i++){
//         printf("%d", bufferQueue[i].fragmentOffset);
//     }
//     printf("\n");
// }

// int isDuplicate(struct Packet* bufferQueue, struct Packet packet, int *inOrder){
//     int i;
//     //whether used?
//     // if(inOrder)
//     //whether in buffre?
//     for(i=0;i<5;i++){
//         if(packet.fragmentOffset == bufferQueue[i].fragmentOffset)
//             return i;
//     }
//     return -1;
// }

// void enqueuePacket(struct Packet *bufferQueue, struct Packet packet){
//     int i;
//     for(i=0; i<5; i++){
//         if(bufferQueue[i].fragmentOffset == -1){
//             bufferQueue[i] = packet;
//             break;
//         }
//     }
//     if(i == 5){
//         printf("Packet Dropped..Since Queue is full\n");
//         exit(0);
//     }
//     // return bufferQueue;
// }

int isFull(struct Packet *bufferQueue, int rear, int front){
    if((rear-front+1) == 5)
        return 1;
    return 0;
}

int isEmpty(struct Packet *bufferQueue, int rear, int front){
    if((rear-front+1) == 0)
        return 1;
    return 0;
}

void enQueue(struct Packet fragment, struct Packet *bufferQueue, int rear, int front){
    if(isFull(bufferQueue, rear, front)){
        printf("Buffer size exceeds...Packets dropped...");
        exit(0);
    }
    bufferQueue[(++rear)%5] = fragment;
}

struct Packet deQueue(struct Packet *bufferQueue, int rear, int front){
    if(isEmpty(bufferQueue, rear, front)){
        printf("Nothing to read...");
        exit(0);
    }
    return bufferQueue[(front++)%5];
}

int isDestinedFragment(struct Packet fragment){
    if(fragment.fragmentOffset == expectedFragment)
        return 1;
    return 0;
}

void deFragment(struct Packet defragmentedPacket, struct Packet fragment){
    strcat(defragmentedPacket.data, fragment.data);
    expectedFragment++;
}

int isNextFragmentInDS(struct Packet *processedFragment, int expectedFragment){
    for(int i=0; i<=processedFragmentCount; i++)
        if(processedFragment[i].fragmentOffset == expectedFragment)
            return i;
    return -1;
}

int yetToDeFragment(struct Packet fragment){
    if(fragment.fragmentOffset > expectedFragment)
        return 1;
    return 0;
}

int isDuplicate(struct Packet *processedFragment, struct Packet fragment){
    for(int i=0; i<=processedFragmentCount; i++)
        if(processedFragment[i].fragmentOffset == fragment.fragmentOffset)
            return 1;
    return 0;
}

void storeInDS(struct Packet *processedFragment, struct Packet fragment){
    if(yetToDeFragment(fragment))
        if(!isDuplicate(processedFragment, fragment)){
            for(int i=0; i<=processedFragmentCount; i++)
                if(processedFragment[i].fragmentOffset == -1){
                    processedFragment[i] = fragment;
                    return;
                }
            processedFragment[++processedFragmentCount] = fragment;
        }
            
}

int main(){

    struct Packet fragment;
    struct Packet defragmentedPacket;

    strcpy(defragmentedPacket.data,"");

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int clientSocket = connectSocket(serverSocket);
    if(clientSocket < 0)
        printf("%d",clientSocket);


    // Drops, if out of order...

    // int readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);

    // int flag;
    // while(readStatus){
    //     if(packetCount == packet.fragmentOffset){
    //         strcat(defragmentedPacket.data, packet.data);
    //         packetCount++;            
    //         flag = 1;
    //     }
    //     else{
    //         printf("Packets out of order :)\n");
    //         flag = 0;
    //         break;
    //     }
    //     readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    // }

    // if(flag == 1){
    //     defragmentedPacket = setHeader(packet);
    //     printDefragmentedPacket(defragmentedPacket);
    // }


     //Re-ordering.. when received..

    struct Packet bufferQueue[5];
    struct Packet processedFragment[5];

    int readStatus = recv(clientSocket, (struct Packet*)&fragment, sizeof(fragment), 0);

    while(readStatus){
        enQueue(fragment, bufferQueue, rear, front);
        fragment = deQueue(bufferQueue, rear, front);
        if(isDestinedFragment(fragment)){
            deFragment(defragmentedPacket, fragment);
            int index = isNextFragmentInDS(processedFragment, expectedFragment);
            while(index >= 0){
                deFragment(defragmentedPacket, processedFragment[index]);
                processedFragment[index].fragmentOffset = -1;
                index = isNextFragmentInDS(processedFragment, expectedFragment);
            }
        }else{
            storeInDS(processedFragment, fragment);
        }
        int readStatus = recv(clientSocket, (struct Packet*)&fragment, sizeof(fragment), 0);
    }

    printf("\n\n%s\n", defragmentedPacket.data);

    close(clientSocket);
    return 0;

}

    //Re-ordering.. when received

    // struct Packet bufferQueue[5];
    // for(int i=0; i<5; i++)
    //     bufferQueue[i].fragmentOffset = -1;
    // int noOfFragments = (int)ceil((float)(fragment.totalLength - fragment.headerLength)/(float)(MTU - fragment.headerLength));
    // bool processedFragmentList[noOfFragments];
    // for(int i=0; i<noOfFragments; i++)
    //     processedFragmentList[i] = false;
    // int hitIndex = -1;
    // int queueIndex = 0;

    // while(readStatus){
    //     if(isProcessed(packet, bufferQueue)){
    //         int index = isDuplicate(bufferQueue, packet, processedFragments);
    //         if(index != -1 ){
    //             if(packet.fragmentOffset == packetCount){
    //                 strcat(defragmentedPacket.data, bufferQueue[index].data);
    //                 inOrder[packetCount]=PacketCount;
    //                 bufferQueue[index].fragmentOffset = -1;
    //                 packetCount++;
    //             }
    //         }else{
    //             enqueuePacket(bufferQueue,packet);
    //         }
    //     }
    //     printQ(bufferQueue);
    //     readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    // }



    // int payloadSize = packet.totalLength - packet.headerLength;
    // struct Packet bufferQueue[5];
    // bufferQueue = (struct Packet*)calloc(10,sizeof(struct Packet));
    // int queueIndex = 0;
    // int hitIndex = -1;

    // while(readStatus){
    //     // printFragmentedPackets(packet);
    //     if(packetCount == packet.fragmentOffset){
    //         strcat(defragmentedPacket.data, packet.data);
    //         printf("%s\n", defragmentedPacket.data);
    //         packetCount++;
    //     }else{
    //         if((hitIndex = (isInQueue(bufferQueue, packetCount)))){
    //             strcat(defragmentedPacket.data, bufferQueue[hitIndex].data);
    //             printf("%s\n", defragmentedPacket.data);
    //             packetCount++;
    //             bufferQueue[hitIndex] = packet;
    //         }else{
    //             bufferQueue[queueIndex++] = packet;
    //         }
    //     }
    //     readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    // }

    // while(strlen(defragmentedPacket.data) < payloadSize){
    //     if((hitIndex = (isInQueue(bufferQueue, packetCount)))){
    //         strcat(defragmentedPacket.data, bufferQueue[hitIndex].data);
    //         packetCount++;
    //     }else{
    //         break;
    //     }
    // }





    // while(readStatus){
    //     if(isOrder(packetCount, packet)){
    //         strcat(defragmentedPacket.data, packet.data);
    //         packetCount++;
    //     }else{
    //         hitIndex = isInQueue(bufferQueue, packetCount);
    //         if(hitIndex > 0){
    //             strcat(defragmentedPacket.data, bufferQueue[hitIndex].data);
    //             packetCount++;
    //             bufferQueue[hitIndex] = packet;
    //         }else{
    //             if(isDuplicate(bufferQueue, packet) == 0)
    //                 bufferQueue[queueIndex++] = packet;
    //         }
    //     }

    //     printQ(bufferQueue);

    //     readStatus = recv(clientSocket, (struct Packet*)&packet, sizeof(packet), 0);
    // }


        // if(isOrder(packetCount, packet)){
        //     strcat(defragmentedPacket.data, packet.data);
        //     packetCount++;
        // }else{
        //     hitIndex = isInQueue(bufferQueue, packetCount);
        //     if(hitIndex > 0){
        //         strcat(defragmentedPacket.data, bufferQueue[hitIndex].data);
        //         packetCount++;
        //         bufferQueue[hitIndex] = packet;
        //     }else{
        //         if(isDuplicate(bufferQueue, packet) == 0)
        //             bufferQueue[queueIndex++] = packet;
        //     }
        // }