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
    char node[10][10];  // hard code 
    int value[10];
} ManagerMsg;


typedef struct __NodeInfo{
    char nodeID[20];  // hard code
    int numVNodes;
} NodeInfo;

typedef struct __NodeInfoPacket{
    int type;  // 8: request from client, 9: request from node
    int size;  // size of the returned nodes
    NodeInfo nodes[100];  // hard code
} NodeInfoPacket;



typedef struct __PacketHead {
    int type;               // 0 for client message, 1 for node message
    unsigned int timpStamp; // Time stamp of the value
    unsigned int seq;       // sequence number of the packet
    unsigned int ack;       // ack = seq + 1, succeeds, otherwise fails.
    unsigned int size;      // size of the value
    int rank;               // rank of the virtual node w.r.t. the storage node it belongs to, -1 for require ring
} PacketHead;

typedef struct __Packet {
    PacketHead head;
    char key[20];
    char value[1024];
} Packet;

#define ROOT "/tmp/"
#define MANAGER "/tmp/manager.domain"
#define MANAGERID "manager.domain"

#endif