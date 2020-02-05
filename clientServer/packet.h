#ifndef flag
    #define flag
        typedef struct{
            int DF;
            int MF;
        }IPFlag;
#endif

#ifndef frameFormat
    #define frameFormat
        typedef struct{
            int version; 
            int headerLength;
            int totalLength;
            IPFlag ipflag;
            int fragmentOffset;
            // char* sourceAddress;
            // char* destinationAddress;
            char data[200];
        }Packet;
#endif