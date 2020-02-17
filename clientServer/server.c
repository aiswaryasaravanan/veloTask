//Receiver who defragment the fragments

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include "serverPrototype.h"

#define PORT1 8080
#define BUFFERSIZE 15

typedef struct
{
    int clientId;
    int threadId;
} ClientThread;

extern struct sockaddr_in serverAddress;

ClientThread clientThread[15];          //struct that associate client with the thread
Packet mainQueue[BUFFERSIZE];     

pthread_mutex_t bufferLock;             //lock for client specific queue
pthread_mutex_t clientThreadCounterLock;

int rear = -1;                          // counter variable for client specific buffer queue
int front = -1;                         // counter variable for client specific buffer queue
int rearMain = -1;                      // counter variable for main buffer queue
int frontMain = -1;                     // counter variable for main buffer queue
int processedFragmentCount = -1;        // counter variable for DS that store future fragments
int packetCount = -1;                   // counter for the resource that associate client with packet
int clientThreadCounter = -1;           // counter for the resource that associate client with thread

Packet setHeader(Packet defragmentedPacket, Packet fragment)
{
    defragmentedPacket.version = fragment.version;
    defragmentedPacket.identification = fragment.identification;
    defragmentedPacket.headerLength = fragment.headerLength;
    defragmentedPacket.totalLength = defragmentedPacket.headerLength + strlen(defragmentedPacket.data);
    defragmentedPacket.fragmentOffset = 0;
    defragmentedPacket.sourceAddress = fragment.sourceAddress;
    defragmentedPacket.ipflag.DF = 0;
    defragmentedPacket.ipflag.MF = 0;
    return defragmentedPacket;
}

int isNewPacket(PacketSpecific *packetSpecific, Packet packet)
{
    for (int i = 0; i <= packetCount; i++)
        if (packet.identification == packetSpecific[i].packet.identification)
        {
            printf("%d id not a new packet\n", packet.identification);
            return 0;
        }
    printf("%d is a new packet\n", packet.identification);
    return 1;
}

void addEntry(PacketSpecific *packetSpecific, Packet packet)
{
    int index = ++packetCount;
    // for(int i=0; i<packetCount; i++)
    //     if(packetSpecific[i].packet.identification == -1){
    //         index = i;
    //         packetCount--;
    //         break;
    //     }
    // packetSpecific[index].packet = packet;
    strcpy(packetSpecific[index].packet.data, "");
    packetSpecific[index].packet = setHeader(packetSpecific[index].packet, packet);
    packetSpecific[index].expectedFragment = 0;
    // packetSpecific[index].timer = 15;
    packetSpecific[index].isDone = 0;
}

PacketSpecific *getEntry(PacketSpecific *packetSpecific, Packet packet)
{
    int i = 0;
    for (; i <= packetCount; i++)
        if (packetSpecific[i].packet.identification == packet.identification)
            break;
    return &packetSpecific[i];
}

//to print the final defragmented packet
void printDefragmentedPacket(Packet defragmentedPacket)
{
    printf("\n\n\nversion:%d\t", defragmentedPacket.version);
    printf("packetIdentification:%d\t", defragmentedPacket.identification);
    printf("fragmentOffset:%d\t", defragmentedPacket.fragmentOffset);
    printf("SourceAddress:%d\n",defragmentedPacket.sourceAddress);
    printf("headerLength:%d\n", defragmentedPacket.headerLength);
    printf("Data:%s\t\t", defragmentedPacket.data);
    printf("totalLength:%d\n", defragmentedPacket.totalLength);
    printf("DF:%d\t", defragmentedPacket.ipflag.DF);
    printf("MF:%d\n\n\n", defragmentedPacket.ipflag.MF);
}

//check whether the fragment is the required fragment
int isDestinedFragment(Packet fragment, int expectedFragment)
{
    if (fragment.fragmentOffset == expectedFragment)
    {
        printf("%d is a destinedFragment\n", fragment.fragmentOffset);
        return 1;
    }
    printf("%d is not a destinedFragment\n", fragment.fragmentOffset);
    return 0;
}

//This will append the fragment data to the final defragmented packet's data
PacketSpecific *deFragment(PacketSpecific *currentEntry, Packet fragment)
{
    strcat(currentEntry->packet.data, fragment.data);
    printf("defragmented packet data:%s\n", currentEntry->packet.data);

    currentEntry->timer = 15;
    currentEntry->packet.totalLength = currentEntry->packet.headerLength + (int)strlen(currentEntry->packet.data);
    currentEntry->expectedFragment = (int)strlen(currentEntry->packet.data);
    return currentEntry;
}

int isNextFragmentInDS(Packet *processedFragment, Packet fragment, PacketSpecific *currentEntry)
{
    if (currentEntry->packetSize == currentEntry->packet.totalLength)
    {
        printDefragmentedPacket(currentEntry->packet);
        currentEntry->isDone = 1;
        return -1;
    }
    for (int i = 0; i <= processedFragmentCount; i++)
        if (processedFragment[i].identification == fragment.identification && processedFragment[i].fragmentOffset == currentEntry->expectedFragment)
        {
            printf("%d - %d is in DS\n", fragment.identification, currentEntry->expectedFragment);
            return i;
        }
    printf("%d - %d is not in DS\n", fragment.identification, currentEntry->expectedFragment);
    return -1;
}

//check if a fragment is a future fragment
int yetToDeFragment(Packet fragment, int expectedFragment)
{
    if (fragment.fragmentOffset > expectedFragment)
    {
        printf("%d - %d is yet to defragment\n", fragment.identification, fragment.fragmentOffset);
        return 1;
    }
    printf("%d - %d is already defragmented\n", fragment.identification, fragment.fragmentOffset);
    return 0;
}

//check the fragment redundancy in the datastructure
int isDuplicate(Packet *processedFragment, Packet fragment)
{
    for (int i = 0; i <= processedFragmentCount; i++)
        if (processedFragment[i].identification == fragment.identification && processedFragment[i].fragmentOffset == fragment.fragmentOffset)
        {
            printf("%d - %d is a duplicate fragment\n", fragment.identification, fragment.fragmentOffset);
            return 1;
        }
    printf("%d - %d is not a duplicate fragment\n", fragment.identification, fragment.fragmentOffset);
    return 0;
}

//if the fragment dequeued from the queue is not the destined fragment
//and if its future fragment and if its not redundant
//then it will be stored in a seperate datastructure
void storeInDS(Packet *processedFragment, Packet fragment, int expectedFragment)
{
    if (yetToDeFragment(fragment, expectedFragment))
        if (!isDuplicate(processedFragment, fragment))
        {
            for (int i = 0; i <= processedFragmentCount; i++)
                if (processedFragment[i].fragmentOffset == -1)
                {
                    processedFragment[i] = fragment;
                    return;
                }
            processedFragment[++processedFragmentCount] = fragment;
            printf("%d - %d stored in DS\n", fragment.identification, fragment.fragmentOffset);
        }
}

void printPackets(PacketSpecific *packetSpecific)
{
    for (int i = 0; i <= packetCount; i++)
    {
        if (packetSpecific[i].isDone != 1)
        {
            printDefragmentedPacket(packetSpecific[i].packet);
        }
    }
}

int isNewClient(int clientId)
{
    for (int i = 0; i < clientThreadCounter; i++)
        if (clientThread[clientThreadCounter].clientId == clientId)
            return 0;
    return 1;
}

pthread_t associateClientWithNewThread(int clientId)
{
    pthread_t clientThreadId;

    pthread_mutex_lock(&clientThreadCounterLock);

    clientThreadCounter++;
    clientThread[clientThreadCounter].clientId = clientId;
    clientThread[clientThreadCounter].threadId = clientThreadId;

    pthread_mutex_unlock(&clientThreadCounterLock);

    return clientThread[clientThreadCounter].threadId;
}

void *receiveAndReorder(void *argv)
{
    Packet bufferQueue[15];          //thread specific queue
    Packet processedFragment[5];            //Data structure to store future fragments - thread specific

    PacketSpecific packetSpecific[15];
    PacketSpecific *currentEntry;

    Packet packet;

    while (1)
    {
        while (isEmpty(bufferQueue, &rear, &front) == 0)
        {
            packet = deQueue(bufferQueue, &rear, &front);

            if (isNewPacket(packetSpecific, packet))
                addEntry(packetSpecific, packet);

            currentEntry = getEntry(packetSpecific, packet);

            if (currentEntry->isDone == 1) //on receiving the duplicate fragment of already defragmented packet
                continue;

            if (packet.ipflag.DF == 1)
                currentEntry->packetSize = packet.fragmentOffset + packet.totalLength;

            if (isDestinedFragment(packet, currentEntry->expectedFragment))
            {
                currentEntry = deFragment(currentEntry, packet);
                int index = isNextFragmentInDS(processedFragment, packet, currentEntry);
                while (index >= 0)
                {
                    currentEntry = deFragment(currentEntry, processedFragment[index]);
                    processedFragment[index].fragmentOffset = -1;
                    index = isNextFragmentInDS(processedFragment, packet, currentEntry);
                }
            }
            else
            {
                storeInDS(processedFragment, packet, currentEntry->expectedFragment);
            }
        }
        // suspendCurrentThread();
        sleep(1);
    }

    return NULL;
}

void *readFromClients(void *argv)
{
    int *clientSocket = (int *)argv;
    Packet packet;
    int readStatus = recv(clientSocket, (struct Packet *)&packet, sizeof(packet), 0);
    enQueue(packet, mainQueue, &rearMain, &frontMain); //enqueue packets to main queue (across clients)
    return NULL;
}

void *assignee(void *argv)
{
    pthread_t client;
    Packet packet;
    packet = deQueue(mainQueue, &rearMain, &frontMain);
    if (isNewClient(packet.sourceAddress))
    {
        client = associateClientWithNewThread(packet.sourceAddress);
        pthread_create(&client, NULL, receiveAndReorder, (void *)&packet);
    }

    // enQueue(packet, bufferQueue, &rear, &front);       // updating child thread queue from parent thread...?   :(

    return NULL;
}

int main()
{

    pthread_t readingThread;
    pthread_t deQueueThread;

    int serverSocket = 0;
    serverSocket = listenSocket(PORT1);
    int clientSocket = 0;

    pthread_mutex_init(&clientThreadCounterLock, NULL);
    pthread_mutex_init(&bufferLock, NULL);

    while (1)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&serverAddress, (socklen_t *)&serverAddress);
        if (clientSocket < 0)
        {
            printf("%d", clientSocket);
            exit(0);
        }

        pthread_create(&readingThread, NULL, readFromClients, (void *)&clientSocket);
        pthread_create(&deQueueThread, NULL, assignee, NULL);
    }

    pthread_mutex_destroy(&bufferLock);
    pthread_mutex_destroy(&clientThreadCounterLock);

    close(clientSocket);
    return 0;
}