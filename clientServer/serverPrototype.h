#include "packet.h"
#include "packetSpecificInfo.h"

int connectSocket(int, int);
int listenSocket(int);
void *receiveAndReorder(void *);
// int initServer(int, int);
int isNewPacket(PacketSpecific *, Packet);
void addEntry(PacketSpecific *, Packet);
PacketSpecific *getEntry(PacketSpecific *, Packet);
Packet setHeader(Packet, Packet);
void printPackets(PacketSpecific *);
void printDefragmentedPacket(Packet);
int isFull(Packet *, int, int);
int isEmpty(Packet *, int, int);
Packet topOfQueue(Packet *, int *, int *);
void enQueue(Packet, Packet *, int *, int *);
Packet deQueue(Packet *, int *, int *);
int isDestinedFragment(Packet, int);
PacketSpecific *deFragment(PacketSpecific *, Packet);
int isNextFragmentInDS(Packet *, Packet, PacketSpecific *);
int yetToDeFragment(Packet, int);
int isDuplicate(Packet *, Packet);
void storeInDS(Packet *, Packet, int);