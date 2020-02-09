#include "packet.h"
#include "packetSpecificInfo.h"

int connectSocket(int, int);
int acceptSocket(int, int);
// int initServer(int, int);
int isNewPacket(Packet);
void addEntry(Packet);
PacketSpecific getEntry(Packet);
Packet setHeader(Packet, Packet);
void printDefragmentedPacket(Packet);
int isFull(Packet*, int, int);
int isEmpty(Packet*, int, int);
int enQueue(Packet, Packet*, int, int);
Packet deQueue(Packet*, int, int);
int isDestinedFragment(Packet, int);
Packet deFragment(PacketSpecific, Packet);
int isNextFragmentInDS(Packet*, int, int);
int yetToDeFragment(Packet, int);
int isDuplicate(Packet*, Packet);
void storeInDS(Packet*, Packet, int);



