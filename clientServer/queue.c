#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

//verify the bufferSize at receiver side.. since the speed at which the sender is sending
// and the speed at which the receiver is receiving differs...
int isFull(Packet *bufferQueue, int rear, int front)
{
    if ((front == 0 && rear == 15 - 1) || (front == rear + 1))
        return 1;
    return 0;
}

int isEmpty(Packet *bufferQueue, int rear, int front)
{
    // if((rear-front+1) == 0)
    if (front == -1)
        return 1;
    return 0;
}

//Enqueue the fragment into the buffer(from the socket) which will later be used by the receiver
void enQueue(Packet fragment, Packet *bufferQueue, int *rear, int *front)
{
    printf("Rear before enqueue :%d\n", *rear);
    if (isFull(bufferQueue, *rear, *front))
    {
        printf("Buffer size exceeds...Packets dropped...\n");
        exit(0);
    }
    if (*front == -1)
    {
        *front = 0;
        *rear = 0;
    }
    else
    {
        if (*rear == 15 - 1)
            *rear = 0;
        else
            *rear = (*rear + 1) % 15;
    }
    bufferQueue[*rear] = fragment;
}

Packet deQueue(Packet *bufferQueue, int *rear, int *front)
{
    if (isEmpty(bufferQueue, *rear, *front))
    {
        printf("Nothing to read...\n");
        exit(0);
    }
    Packet packet = bufferQueue[*front];
    if (*front == *rear)
    {
        *front = -1;
        *rear = -1;
    }
    else
    {
        if (*front == 15 - 1)
            *front = 0;
        else
            *front = (*front + 1) % 15;
    }
    return packet;
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