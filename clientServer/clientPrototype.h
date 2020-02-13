#include "packet.h"
        int connectSocket(int, int);
        int listenSocket(int);
Packet fragmentPacket(Packet, int);
int isYetToSend(int *, int);
void updateCheckList(int *, int, int);
int generatePacketId();
Packet setHeader(Packet, int, int, int, char *, int, int, int);
void shuffleAndSend(int, Packet *, int);
