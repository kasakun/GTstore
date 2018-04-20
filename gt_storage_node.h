#ifndef GT_STORAGE_NODE_H
#define GT_STORAGE_NODE_H

#include "gt_object.h"


using  Pair = std::unordered_map<ObjectKeyType, ObjectValueType>;
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

private:
    std::string vnodeID;                                       // its own ID
    std::string nodeID;                                        // the ID of the actual storage node
    int rank;                                                  // the rank among the virtual nodes of the same storage node
    Pair* data;  // the actual key-value pairs
};


class StorageNode{
public:
    StorageNode(const std::string& nodeID_ = "unnamed_node", const int& numVirtualNodes_ = 1);
    ~StorageNode();
    std::string getNodeID() const;
    int getNumVirtualNodes() const;
    std::vector<VirtualNode> getVirtualNodes() const;
    bool writeToVNode(int rank, ObjectKeyType key, ObjectValueType value);
    void sendBack(Packet& p, unsigned int seq);
    Packet unPack(char* buf);
    void run();
    
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
    std::vector<Pair::iterator> store;
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