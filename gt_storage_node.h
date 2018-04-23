#ifndef GT_STORAGE_NODE_H
#define GT_STORAGE_NODE_H

#include "gt_object.h"
#include "consistent_hash_ring.h"


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
    bool writeKeyVersionPair(ObjectKeyType key, ObjectVersionType version);
    bool readKeyVersionPair(ObjectKeyType key, ObjectVersionType& version);

    
private:
    std::string vnodeID;                                       // its own ID
    std::string nodeID;                                        // the ID of the actual storage node
    int rank;                                                  // the rank among the virtual nodes of the same storage node
    Map* data;  // the actual key-value pairs
    std::unordered_map<ObjectKeyType, ObjectVersionType>* versions;
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


class StorageNode{
public:
    StorageNode(const std::string& nodeID_ = "unnamed_node", const int& numVirtualNodes_ = 1);
    ~StorageNode();
    std::string getNodeID() const;
    int getNumVirtualNodes() const;
    std::vector<VirtualNode> getVirtualNodes() const;
    
    bool writeToLocalVNode(int rank, ObjectKeyType key, ObjectValueType value);
    bool readFromLocalVNode(int rank, ObjectKeyType& key, ObjectValueType& value);
    
    
    bool init();
    
    
    Packet unPack(char* buf);      //unpack buf to the packet
    bool createListenSocket(int& nodefd);
    bool receiveMessage(int& nodefd, int& nodeAccept, char* buf, Packet& p);

    bool managerHandler(int& nodefd, int& nodeAccept, Packet& p);
    bool nodeHandler(int& nodefd, int& nodeAccept, Packet& p);
    bool clientHandler(int& nodefd, int& nodeAccept, Packet& p);
    bool clientCoordinator(int& nodefd, int& nodeAccept, Packet& p);
    void run();

    /*
     * node communication
     */
    bool write(Packet& p);  // write to a local virtual node
    bool read(Packet& p);  // read from a local virtual node
    void writeBackPack(Packet& p); //sendback ack, replace value with wirte nodeID
    void readBackPack(Packet& p);  //sendback ack and data

    /*
     * node communication
     */
    bool writeToNodes(Packet& p, std::vector<std::pair<std::string, int>> preferenceList);
    bool readFromNodes(Packet& p, std::vector<std::pair<std::string, int>> list);

    // find the coordinator given a key
    std::pair<std::string, int> findCoordinator(ObjectKeyType key);
    
    std::string nodeID;   // node ID is used to identify a storage node
    int numVirtualNodes;  // number of virtual nodes of this storage node
    
private:
    std::string constructVirtualNodeID(int i);
    void addNodesToRing(std::vector<StorageNode>& nodes);
    std::vector<std::pair<std::string, int>> getPreferenceList(const ObjectKeyType& objectKey, int sizeList);  // preference list does not include coordinator
    
    
    
    std::vector<VirtualNode> vnodes;

    /* data store
     * store -> store0 --- (key, value)
     *                 --- (key, value)
     *       -> store1
     */
    //std::vector<Map> store;  // no need to use store -Yaohong
    ConsistentHashRing<VirtualNode,VirtualNodeHasher> ring;
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