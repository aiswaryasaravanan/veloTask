#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

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