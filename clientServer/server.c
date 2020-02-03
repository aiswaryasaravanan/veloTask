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

// struct Packet defragmentedPacket;

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

// void printQueue(struct Packet *bufferQueue){
//     if (rear >= front) 
//         for (int i = front; i <= rear; i++) 
//             printf("%d ",bufferQueue[i].fragmentOffset); 
//     else
//     { 
//         for (int i = front; i < 5; i++) 
//             printf("%d ", bufferQueue[i].fragmentOffset); 
  
//         for (int i = 0; i <= rear; i++) 
//             printf("%d ", bufferQueue[i].fragmentOffset); 
//     }
//     printf("\n");
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
        printf("Buffer size exceeds...Packets dropped...\n");
        exit(0);
    }
    bufferQueue[(++rear)%5] = fragment;
    // return rear;
}

struct Packet deQueue(struct Packet *bufferQueue, int rear, int front){
    if(isEmpty(bufferQueue, rear, front)){
        printf("Nothing to read...\n");
        exit(0);
    }
    return bufferQueue[(front++)%5];
}

int isDestinedFragment(struct Packet fragment){
    if(fragment.fragmentOffset == expectedFragment){
        // printf("%d is a destinedFragment\n",fragment.fragmentOffset);
        return 1;
    }
    // printf("%d is not a destinedFragment\n",fragment.fragmentOffset);
    return 0;
}

struct Packet deFragment(struct Packet defragmentedPacket, struct Packet fragment){
    // printf("defragmenting %d\n",fragment.fragmentOffset);
    strcat(defragmentedPacket.data, fragment.data);
    // printf("defragmnted packet data:%s\n", defragmentedPacket.data);
    expectedFragment++;
    return defragmentedPacket;
}

int isNextFragmentInDS(struct Packet *processedFragment, int expectedFragment){
    for(int i=0; i<=processedFragmentCount; i++)
        if(processedFragment[i].fragmentOffset == expectedFragment){
            // printf("%d is in DS\n",expectedFragment);
            return i;
        }
    // printf("%d is not in DS\n",expectedFragment);   
    return -1;
}

int yetToDeFragment(struct Packet fragment){
    if(fragment.fragmentOffset > expectedFragment){
        // printf("%d is yet to defragment\n",fragment.fragmentOffset);
        return 1;
    }
    // printf("%d is already defragmented\n",fragment.fragmentOffset);
    return 0;
}

int isDuplicate(struct Packet *processedFragment, struct Packet fragment){
    for(int i=0; i<=processedFragmentCount; i++)
        if(processedFragment[i].fragmentOffset == fragment.fragmentOffset){
            // printf("%d is a duplicate fragment\n",fragment.fragmentOffset);
            return 1;
        }
    // printf("%d is not a duplicate fragment\n",fragment.fragmentOffset);
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

    strcat(defragmentedPacket.data,"");

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

        // enQueue(fragment, bufferQueue, rear, front);     //since..sender and receiver are at different speed
        // fragment = deQueue(bufferQueue, rear, front);
        if(isDestinedFragment(fragment)){
            defragmentedPacket = deFragment(defragmentedPacket, fragment);
            int index = isNextFragmentInDS(processedFragment, expectedFragment);
            while(index >= 0){
                defragmentedPacket = deFragment(defragmentedPacket, processedFragment[index]);
                processedFragment[index].fragmentOffset = -1;
                index = isNextFragmentInDS(processedFragment, expectedFragment);
            }
        }else
            storeInDS(processedFragment, fragment);
        
        readStatus = recv(clientSocket, (struct Packet*)&fragment, sizeof(fragment), 0);

    }

    defragmentedPacket = setHeader(fragment, defragmentedPacket.data);
    printDefragmentedPacket(defragmentedPacket);

    close(clientSocket);
    return 0;

}

