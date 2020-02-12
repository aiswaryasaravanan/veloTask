//Receiver who defragment the fragments

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include "serverPrototype.h"
// #include "packet.h"
// #include "packetSpecificInfo.h"

#define PORT1 8080

int rear = -1;
int front = 0;
int processedFragmentCount = -1;
int packetCount = -1;

PacketSpecific packetSpecific[15];
PacketSpecific *currentEntry;

Packet setHeader(Packet defragmentedPacket, Packet fragment)
{
    defragmentedPacket.version = fragment.version;
    defragmentedPacket.identification = fragment.identification;
    defragmentedPacket.headerLength = fragment.headerLength;
    defragmentedPacket.totalLength = defragmentedPacket.headerLength + strlen(defragmentedPacket.data);
    defragmentedPacket.fragmentOffset = 0;
    defragmentedPacket.ipflag.DF = 0;
    defragmentedPacket.ipflag.MF = 0;
    return defragmentedPacket;
}

int isNewPacket(Packet packet)
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

void addEntry(Packet packet)
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
    packetSpecific[index].timer = 15;
}

PacketSpecific *getEntry(Packet packet)
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
    printf("defragmenting %d\n", fragment.fragmentOffset);
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
            // printf("\nPacketID:%d \t PacketData:%s \n", packetSpecific[i].packet.identification, packetSpecific[i].packet.data);
            printDefragmentedPacket(packetSpecific[i].packet);
        }
    }
}

int main()
{

    Packet fragment;

    int serverSocket = 0;
    int clientSocket = connectSocket(serverSocket, PORT1);

    Packet bufferQueue[5];       //queue
    Packet processedFragment[5]; //DS

    //Re-ordering.. when received..
    int readStatus = recv(clientSocket, (struct Packet *)&fragment, sizeof(fragment), 0);

    while (readStatus)
    {
        // rear = enQueue(fragment, bufferQueue, rear, front);     //since..sender and receiver are at different speed
        // fragment = deQueue(bufferQueue, rear, front);

        if (isNewPacket(fragment))
        {
            addEntry(fragment);
        }
        currentEntry = getEntry(fragment);
        if (currentEntry->isDone == 1)
        {
            readStatus = recv(clientSocket, (struct Packet *)&fragment, sizeof(fragment), 0);
            continue;
        }

        // if(currentEntry.timer > 0){
        if (fragment.ipflag.DF == 1)
            currentEntry->packetSize = fragment.fragmentOffset + fragment.totalLength;
        if (isDestinedFragment(fragment, currentEntry->expectedFragment))
        {
            currentEntry = deFragment(currentEntry, fragment);
            int index = isNextFragmentInDS(processedFragment, fragment, currentEntry);
            while (index >= 0)
            {
                currentEntry = deFragment(currentEntry, processedFragment[index]);
                processedFragment[index].fragmentOffset = -1;
                index = isNextFragmentInDS(processedFragment, fragment, currentEntry);
            }
        }
        else
        {
            storeInDS(processedFragment, fragment, currentEntry->expectedFragment);
        }

        readStatus = recv(clientSocket, (struct Packet *)&fragment, sizeof(fragment), 0);
        // }else{
        //     printf("Timer expires...Wait time exceeded...");
        //     // exit(0);
        // }
        // currentEntry.timer--;
    }
    // if(currentPacket.packet.totalLength < currentPacket.packetSize){
    //     printf("Packet loss occurs...\n");
    //     exit(0);
    // }
    printPackets(packetSpecific);

    close(clientSocket);
    return 0;
}