#include "packet.h"


int connectSocket(int, int);
int acceptSocket(int, int);
Packet fragmentPacket(Packet, int);
int isYetToSend(int*, int);
void updateCheckList(int*, int, int);
Packet setHeader(Packet, int, int, char*, int, int, int, int);
void shuffleAndSend(int, Packet*, int);