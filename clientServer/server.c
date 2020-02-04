//Receiver who defragment the fragments

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int rear = -1;
int front = 0;
int expectedFragment = 0;
int processedFragmentCount = -1;

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

int initServer(int);
Packet setHeader(Packet, Packet);
void printDefragmentedPacket(Packet);
int isFull(Packet*, int, int);
int isEmpty(Packet*, int, int);
int enQueue(Packet, Packet*, int, int);
Packet deQueue(Packet*, int, int);
int isDestinedFragment(Packet);
Packet deFragment(Packet defragmentedPacket, Packet fragment);
int isNextFragmentInDS(Packet*, int);
int yetToDeFragment(Packet);
int isDuplicate(Packet*, Packet);
void storeInDS(Packet*, Packet);

//to connect and accept the socket connection with client
int initServer(int serverSocket){
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

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

    int clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddress);
    if(clientSocket < 0){
        printf("%d",clientSocket);
        exit(0);
    }return clientSocket;
}

// void setHeader(Packet deFragmentedPacket, Packet fragment){
//     deFragmentedPacket.version = fragment.version;
//     deFragmentedPacket.headerLength = fragment.headerLength;
//     deFragmentedPacket.totalLength = fragment.totalLength;
//     deFragmentedPacket.fragmentOffset = 0;
//     deFragmentedPacket.ipflag.DF = 0;
//     deFragmentedPacket.ipflag.MF = 0;
// }

Packet setHeader(Packet defragmentedPacket, Packet fragment){
    defragmentedPacket.version = fragment.version;
    defragmentedPacket.headerLength = fragment.headerLength;
    defragmentedPacket.totalLength = fragment.totalLength;
    defragmentedPacket.fragmentOffset = 0;
    defragmentedPacket.ipflag.DF = 0;
    defragmentedPacket.ipflag.MF = 0;
    // strcpy(defragmentedPacket.data,data);
    return defragmentedPacket;
}

//to print the final defragmented packet
void printDefragmentedPacket(Packet defragmentedPacket){
    printf("\n\nversion:%d\t", defragmentedPacket.version);
    printf("fragmentOffset:%d\t", defragmentedPacket.fragmentOffset);
    printf("headerLength:%d\n", defragmentedPacket.headerLength);
    printf("Data:%s\t\t", defragmentedPacket.data);
    printf("totalLength:%d\n", defragmentedPacket.totalLength);
    printf("DF:%d\t", defragmentedPacket.ipflag.DF);
    printf("MF:%d\n", defragmentedPacket.ipflag.MF);
    
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


//verify the bufferSize at receiver side.. since the speed at which the sender is sending 
// and the speed at which the receiver is receiving differs...
int isFull(Packet *bufferQueue, int rear, int front){
    if((rear-front+1) == 5)
        return 1;
    return 0;
}

int isEmpty(Packet *bufferQueue, int rear, int front){
    if((rear-front+1) == 0)
        return 1;
    return 0;
}

//Enqueue the fragment into the buffer(from the socket) which will later be used by the receiver
int enQueue(Packet fragment, Packet *bufferQueue, int rear, int front){
    if(isFull(bufferQueue, rear, front)){
        printf("Buffer size exceeds...Packets dropped...\n");
        exit(0);
    }
    bufferQueue[(++rear)%5] = fragment;
    return rear;
}

Packet deQueue(Packet *bufferQueue, int rear, int front){
    if(isEmpty(bufferQueue, rear, front)){
        printf("Nothing to read...\n");
        exit(0);
    }
    return bufferQueue[(front++)%5];
}

//check whether the fragment is the required fragment
int isDestinedFragment(Packet fragment){
    if(fragment.fragmentOffset == expectedFragment){
        printf("%d is a destinedFragment\n",fragment.fragmentOffset);
        return 1;
    }
    printf("%d is not a destinedFragment\n",fragment.fragmentOffset);
    return 0;
}

//This will append the fragment data to the final defragmented packet's data
Packet deFragment(Packet defragmentedPacket, Packet fragment){
    printf("defragmenting %d\n",fragment.fragmentOffset);
    strcat(defragmentedPacket.data, fragment.data);
    printf("defragmnted packet data:%s\n", defragmentedPacket.data);
    expectedFragment++;
    return defragmentedPacket;
}

int isNextFragmentInDS(Packet *processedFragment, int expectedFragment){
    for(int i=0; i<=processedFragmentCount; i++)
        if(processedFragment[i].fragmentOffset == expectedFragment){
            printf("%d is in DS\n",expectedFragment);
            return i;
        }
    printf("%d is not in DS\n",expectedFragment);   
    return -1;
}

//check if a fragment is a future fragment
int yetToDeFragment(Packet fragment){
    if(fragment.fragmentOffset > expectedFragment){
        printf("%d is yet to defragment\n",fragment.fragmentOffset);
        return 1;
    }
    printf("%d is already defragmented\n",fragment.fragmentOffset);
    return 0;
}

//check the fragment redundancy in the datastructure
int isDuplicate(Packet *processedFragment, Packet fragment){
    for(int i=0; i<=processedFragmentCount; i++)
        if(processedFragment[i].fragmentOffset == fragment.fragmentOffset){
            printf("%d is a duplicate fragment\n",fragment.fragmentOffset);
            return 1;
        }
    printf("%d is not a duplicate fragment\n",fragment.fragmentOffset);
    return 0;
}

//if the fragment dequeued from the queue is not the destined fragment 
//and if its future fragment and if its not redundant 
//then it will be stored in a seperate datastructure
void storeInDS(Packet *processedFragment, Packet fragment){
    if(yetToDeFragment(fragment))
        if(!isDuplicate(processedFragment, fragment)){
            for(int i=0; i<=processedFragmentCount; i++)
                if(processedFragment[i].fragmentOffset == -1){
                    processedFragment[i] = fragment;
                    return;
                }
            processedFragment[++processedFragmentCount] = fragment;
            printf("%d stored in DS\n", fragment.fragmentOffset);
        }     
}

int main(){

    Packet fragment;
    Packet defragmentedPacket;

    strcat(defragmentedPacket.data,"");

    // int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int serverSocket = 0;
    int clientSocket = initServer(serverSocket);

    //Re-ordering.. when received..

    Packet bufferQueue[5];
    Packet processedFragment[5];

    int readStatus = recv(clientSocket, (struct Packet*)&fragment, sizeof(fragment), 0);
    while(readStatus && ((int)strlen(defragmentedPacket.data))<(fragment.totalLength-fragment.headerLength)){       //checking MF :(
        // rear = enQueue(fragment, bufferQueue, rear, front);     //since..sender and receiver are at different speed
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

    defragmentedPacket = setHeader(defragmentedPacket, fragment);
    printDefragmentedPacket(defragmentedPacket);

    close(clientSocket);
    return 0;

}