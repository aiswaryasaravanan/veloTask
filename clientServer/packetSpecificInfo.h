#include "packet.h"

#ifndef entry
    #define entry
        typedef struct{
            Packet packet;
            int isDone;
            int packetSize;
            int expectedFragment;
            int timer;
        } PacketSpecific;
#endif

