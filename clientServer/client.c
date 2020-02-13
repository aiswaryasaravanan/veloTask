//Sender who fragments the packet if the packet size is greater than MTU

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <math.h>
#include <time.h> //for srand function
#include "clientPrototype.h"

#define MTU 30
#define VERSION 4
#define HEADERLENGTH 20
#define PORT1 8080

int generatePacketId()
{
    int packetId = rand();
    printf("\nPacket ID: %d \n", packetId);
    return packetId;
}

Packet setHeader(Packet packet, int packetId, int version, int headerLength, char *data, int fragmentOffset, int isFragment, int isLastFragment)
{
    packet.version = version;
    packet.headerLength = headerLength;
    packet.totalLength = strlen(data) + HEADERLENGTH;
    packet.fragmentOffset = fragmentOffset;
    packet.identification = packetId;

    if (isFragment == 0 || isLastFragment == 1)
    { //its a packet...
        packet.ipflag.DF = 1;
        packet.ipflag.MF = 0;
    }
    else
    {
        packet.ipflag.DF = 0;
        packet.ipflag.MF = 1;
    }

    return packet;
}

//cutting down the packet data into fragments as per MTU value using strncpy
Packet fragmentPacket(Packet packet, int fragOffset)
{
    printf("fragmenting....\n");
    Packet fragmentedPacket;
    int isLastFragment = (fragOffset + (MTU - HEADERLENGTH) < strlen(packet.data)) ? 0 : 1;
    strncpy(fragmentedPacket.data, packet.data + fragOffset, (MTU - packet.headerLength));
    fragmentedPacket = setHeader(fragmentedPacket, packet.identification, VERSION, HEADERLENGTH, fragmentedPacket.data, fragOffset, 1, isLastFragment);
    return fragmentedPacket;
}

//check if all the fragments would have been sent
int isYetToSend(int checkList[], int noOfFragments)
{
    for (int i = 0; i < noOfFragments; i++)
        if (checkList[i] != -1)
            return 1;
    return 0;
}

//keep track of whatever fragment sent
void updateCheckList(int checkList[], int randPacket, int noOfFragments)
{
    for (int i = 0; i < noOfFragments; i++)
        if (checkList[i] == randPacket)
        {
            checkList[i] = -1;
            break;
        }
}

void printFragment(Packet fragment)
{
    printf("version:%d\t", fragment.version);
    printf("packetIdentification:%d\t", fragment.identification);
    printf("fragmentOffset:%d\t", fragment.fragmentOffset);
    printf("headerLength:%d\n", fragment.headerLength);
    printf("Data:%s\t", fragment.data);
    printf("totalLength:%d\n", fragment.totalLength);
    printf("DF:%d\t", fragment.ipflag.DF);
    printf("MF:%d\n\n\n", fragment.ipflag.MF);
}

// generate a random number and send the fragments accordingly

void shuffleAndSend(int noOfFragments, Packet *fragmentedPackets, int clientSocket)
{
    srand(time(NULL));
    int randPacket = 0;
    int checkList[noOfFragments];

    for (int i = 0; i < noOfFragments; i++)
        checkList[i] = i;

    while (isYetToSend(checkList, noOfFragments))
    {
        randPacket = rand() % noOfFragments;
        send(clientSocket, (void *)&fragmentedPackets[randPacket], sizeof(fragmentedPackets[randPacket]), 0);
        updateCheckList(checkList, randPacket, noOfFragments);
        printFragment(fragmentedPackets[randPacket]);
    }
}

// To drop some packet..

// void shuffleAndSend(int noOfFragments, Packet *fragmentedPackets, int clientSocket){
//     srand(time(NULL));
//     int index;
//     for(int j=0; j<noOfFragments; j++){
//         index = rand() % noOfFragments;
//         printf("%d\n",index);
//         send(clientSocket, (void *)&fragmentedPackets[index], sizeof(fragmentedPackets[index]), 0);
//     }
// }

int main()
{
    Packet packet;
    Packet fragment;
    Packet fragmentedPackets[100];

    int clientSocket = 0;
    clientSocket = acceptSocket(clientSocket, PORT1);
    int choice = 0;

    int noOfPacket = 1;
    int index = 0;
    int noOfFragments = 0;
    int packetId = 0;

    do
    {
        // reseting
        choice = 0;

        // starts
        packetId = generatePacketId();
        printf("Enter data:\n");
        scanf("%s", packet.data);

        packet = setHeader(packet, packetId, VERSION, HEADERLENGTH, packet.data, 0, 0, 0);

        if (packet.totalLength > MTU)
        {
            printf("Have to fragment this packet...");
            noOfFragments += (int)ceil((float)(strlen(packet.data)) / (float)(MTU - packet.headerLength));

            for (int fragOffset = 0; fragOffset < (packet.totalLength - packet.headerLength); fragOffset += (MTU - packet.headerLength))
                fragmentedPackets[index++] = fragmentPacket(packet, fragOffset);
            // shuffleAndSend(noOfFragments, fragmentedPackets, clientSocket);
        }
        else
        {
            printf("No need to fragment this packet...\n");
            send(clientSocket, (void *)&packet, sizeof(packet), 0);
        }

        noOfPacket--;
        if (noOfPacket == 0)
        {
            shuffleAndSend(noOfFragments, fragmentedPackets, clientSocket);
            index = 0;
            noOfFragments = 0;
            printf("Do you want to continue sending more packets: (1/0)\n");
            scanf("%d", &choice);
            if (choice == 1)
                noOfPacket = 2; // reset to initial size
        }
    } while (noOfPacket > 0 || choice == 1);

    // shuffleAndSend(noOfFragments, fragmentedPackets, clientSocket);
    close(clientSocket);
    return 0;
}
