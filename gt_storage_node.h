#ifndef GT_STORAGE_NODE_H
#define GT_STORAGE_NODE_H

#include "gt_object.h"


using  Map = std::unordered_map<ObjectKeyType, ObjectValueType>;
class VirtualNode{
public:
    VirtualNode(const std::string& vnodeID_, \
                const std::string& nodeID_, \
                const int& rank_);
    ~VirtualNode();
    std::string getVNodeID() const;
    std::string getNodeID() const;
    int getRank() const;
    bool operator==(const VirtualNode& other) const;
    bool writeKeyValuePair(ObjectKeyType key, ObjectValueType value);
    bool readKeyValuePair(ObjectKeyType key, ObjectValueType& value);

private:
    std::string vnodeID;                                       // its own ID
    std::string nodeID;                                        // the ID of the actual storage node
    int rank;                                                  // the rank among the virtual nodes of the same storage node
    Map* data;  // the actual key-value pairs
};


class StorageNode{
public:
    StorageNode(const std::string& nodeID_ = "unnamed_node", const int& numVirtualNodes_ = 1);
    ~StorageNode();
    std::string getNodeID() const;
    int getNumVirtualNodes() const;
    std::vector<VirtualNode> getVirtualNodes() const;
    bool writeToVNode(int rank, ObjectKeyType key, ObjectValueType value);
    bool readVNode(int rank, ObjectKeyType& key, ObjectValueType& value);
    Packet unPack(char* buf);      //unpack buf to the packet
    bool createListenSocket(int& nodefd);
    bool receiveMessage(int& nodefd, int& nodeAccept, char* buf, Packet& p);

    bool managerHandler(int& nodefd, int& nodeAccept, Packet& p);
    bool nodeHandler(int& nodefd, int& nodeAccept, Packet& p);
    bool clientHandler(int& nodefd, int& nodeAccept, Packet& p);

    void run();

    /*
     * node communication
     */
    bool write(Packet& p);
    bool read(Packet& p);
    void writeSendBack(Packet& p); //sendback ack, replace value with wirte node of the operation,
    void readSendBack(Packet& p);  //sendback ack and data

    /*
     * node communication
     */
    bool writeToNodes(Packet& p, std::vector<std::pair<std::string, int>> preferenceList);
    
private:
    std::string constructVirtualNodeID(int i);

    std::string nodeID;   // ID = IP address?
    int numVirtualNodes;  // for better load balancing
    std::vector<VirtualNode> vnodes;

    /* data store
     * store -> store0 --- (key, value)
     *                 --- (key, value)
     *       -> store1
     */
    std::vector<Map> store;
};


class VirtualNodeHasher{
public:
    VirtualNodeHasher();
    ~VirtualNodeHasher();
    std::size_t operator()(const VirtualNode& vnode) const;
    typedef std::size_t ResultType;
private:
    std::hash<std::string> stringHasher;
};

// << overload
inline std::ostream& operator<<(std::ostream& os, const StorageNode& node) {
    os << node.getNodeID();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const VirtualNode& vnode) {
    os << vnode.getVNodeID();
    return os;
}

#endif