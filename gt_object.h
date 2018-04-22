#ifndef GT_OBJECT_H
#define GT_OBJECT_H

#include <string>
#include <vector>

typedef std::string ObjectKeyType;                   // 20B
typedef std::vector<std::string> ObjectValueType;    // 1024B

typedef struct __Msg {
    ObjectKeyType key;
    ObjectValueType value;
} Msg;

// current is ten elements in preference list, can be extended.
typedef struct __ManagerMsg {
    int num;
    char node[10][10];
    int value[10];
} ManagerMsg;

typedef struct __PacketHead {
    int type;               // 0 for client message, 1 for node message
    unsigned int timpStamp; // Time stamp of the value
    unsigned int seq;       // sequence number of the packet
    unsigned int ack;       // ack = seq - 1, succeeds, otherwise fails.
    unsigned int size;      // size of the value
    int rank;               // rank of the virtual node at the node, -1 for require ring
} PacketHead;

typedef struct __Packet {
    PacketHead head;
    char key[20];
    char value[1024];
} Packet;
#endif