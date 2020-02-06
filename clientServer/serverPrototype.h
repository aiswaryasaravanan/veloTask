#include "packet.h"

int connectSocket(int, int);
int acceptSocket(int, int);
// int initServer(int, int);
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