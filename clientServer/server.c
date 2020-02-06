//Receiver who defragment the fragments

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include "serverPrototype.h"

#define PORT1 8080

int rear = -1;
int front = 0;
int expectedFragment = 0;
int processedFragmentCount = -1;
int timer = 15;


Packet setHeader(Packet defragmentedPacket, Packet fragment){
    defragmentedPacket.version = fragment.version;
    defragmentedPacket.identification = fragment.identification;
    defragmentedPacket.headerLength = fragment.headerLength;
    defragmentedPacket.totalLength = defragmentedPacket.headerLength + strlen(defragmentedPacket.data);
    defragmentedPacket.fragmentOffset = 0;
    defragmentedPacket.ipflag.DF = 0;
    defragmentedPacket.ipflag.MF = 0;
    return defragmentedPacket;
}

//to print the final defragmented packet
void printDefragmentedPacket(Packet defragmentedPacket){
    printf("\n\nversion:%d\t", defragmentedPacket.version);
    printf("packetIdentification:%d\t", defragmentedPacket.identification);
    printf("fragmentOffset:%d\t", defragmentedPacket.fragmentOffset);
    printf("headerLength:%d\n", defragmentedPacket.headerLength);
    printf("Data:%s\t\t", defragmentedPacket.data);
    printf("totalLength:%d\n", defragmentedPacket.totalLength);
    printf("DF:%d\t", defragmentedPacket.ipflag.DF);
    printf("MF:%d\n", defragmentedPacket.ipflag.MF);
    
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
    timer = 15;
    printf("defragmnted packet data:%s\n", defragmentedPacket.data);
    defragmentedPacket.totalLength = defragmentedPacket.headerLength +strlen(defragmentedPacket.data);
    expectedFragment+=fragment.totalLength - fragment.headerLength;
    // expectedFragment++;
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

    int serverSocket = 0;
    int clientSocket = connectSocket(serverSocket, PORT1);

    Packet bufferQueue[5];
    Packet processedFragment[5];

    //Re-ordering.. when received..

    int packetSize = INT_MAX;         //since the entire packet size and MTU of sender are unknown...lets have this to keep track of offset
    
    int readStatus = recv(clientSocket, (struct Packet*)&fragment, sizeof(fragment), 0);
    defragmentedPacket = setHeader(defragmentedPacket, fragment);

    while(readStatus && (strlen(defragmentedPacket.data) < packetSize)){

        // rear = enQueue(fragment, bufferQueue, rear, front);     //since..sender and receiver are at different speed
        // fragment = deQueue(bufferQueue, rear, front);


        if(timer > 0){
            if(fragment.ipflag.DF == 1)
                packetSize = fragment.fragmentOffset + fragment.totalLength;

            printf("PacketSize:%d -%d\n",packetSize,defragmentedPacket.totalLength);

            if(isDestinedFragment(fragment)){
                defragmentedPacket = deFragment(defragmentedPacket, fragment);
                int index = isNextFragmentInDS(processedFragment, expectedFragment);
                while(index >= 0){
                    defragmentedPacket = deFragment(defragmentedPacket, processedFragment[index]);
                    processedFragment[index].fragmentOffset = -1;
                    index = isNextFragmentInDS(processedFragment, expectedFragment);
                }
            }else{
                storeInDS(processedFragment, fragment);
            }readStatus = recv(clientSocket, (struct Packet*)&fragment, sizeof(fragment), 0);
        }else{
            printf("Timer expires...Wait time exceeded...");
            exit(0);
        }
        timer--;
    }
    if(defragmentedPacket.totalLength < packetSize){
        printf("Packet loss occurs...\n");
        exit(0);
    }

    defragmentedPacket = setHeader(defragmentedPacket, fragment);
    printDefragmentedPacket(defragmentedPacket);

    close(clientSocket);
    return 0;

}