#include "packet.h"

int initClient(int);
Packet fragmentPacket(Packet, int, int);
int isYetToSend(int*, int);
void updateCheckList(int*, int, int);
Packet setHeader(Packet, int, int);
Packet setFlag (Packet);
void shuffleAndSend(int, Packet*, int);