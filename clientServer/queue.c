#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "packet.h"
#include "packetSpecificInfo.h"

#define BUFFERSIZE 15

extern pthread_mutex_t bufferLock;

//verify the bufferSize at receiver side.. since the speed at which the sender is sending
// and the speed at which the receiver is receiving differs...
int isFull(Packet *queue, int rear, int front)
{
    if ((front == 0 && rear == BUFFERSIZE - 1) || (front == rear + 1))
        return 1;
    return 0;
}

int isEmpty(Packet *queue, int rear, int front)
{
    // if((rear-front+1) == 0)
    if (front == -1)
        return 1;
    return 0;
}

//Enqueue the fragment into the buffer(from the socket) which will later be used by the receiver
void enQueue(Packet packet, Packet *queue, int *rear, int *front)
{
    printf("Rear before enqueue :%d\n", *rear);
    if (isFull(queue, *rear, *front))
    {
        printf("Buffer size exceeds...Packets dropped...\n");
        exit(0);
    }

    pthread_mutex_lock(&bufferLock);

    if (*front == -1)
    {
        *front = 0;
        *rear = 0;
    }
    else
    {
        if (*rear == BUFFERSIZE - 1)
            *rear = 0;
        else
            *rear = (*rear + 1) % BUFFERSIZE;
    }
    queue[*rear] = packet;

    pthread_mutex_unlock(&bufferLock);
}

Packet deQueue(Packet *queue, int *rear, int *front)
{
    if (isEmpty(queue, *rear, *front))
    {
        printf("Nothing to read...\n");
        exit(0);
    }

    pthread_mutex_lock(&bufferLock);

     Packet packet = queue[*front];
    if (*front == *rear)
    {
        *front = -1;
        *rear = -1;
    }
    else
    {
        if (*front == BUFFERSIZE - 1)
            *front = 0;
        else
            *front = (*front + 1) % BUFFERSIZE;
    }

    pthread_mutex_unlock(&bufferLock);

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
