#ifndef GT_STORAGE_NODE_H
#define GT_STORAGE_NODE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "gt_object.h"

class VirtualNode{
public:
    VirtualNode(const std::string& vnodeID_, const std::string& nodeID_):vnodeID(vnodeID_), nodeID(nodeID_){
        data = new std::unordered_map<ObjectKeyType, ObjectValueType>();
    }

    std::string getVNodeID() const{
        return vnodeID;
    }
    
    bool operator==(const VirtualNode& other) const{
        return this->vnodeID == other.vnodeID; 
    }

private:
    std::string vnodeID;  // its own ID
    std::string nodeID;  // the ID of the actual storage node
    std::unordered_map<ObjectKeyType, ObjectValueType>* data;  // the actual key-value pairs
};


class StorageNode{
public:
    StorageNode(const std::string& nodeID_, const int& numVirtualNodes_):nodeID(nodeID_), numVirtualNodes(numVirtualNodes_){
        for(int i = 0; i < numVirtualNodes; ++i){
            std::string vnodeID = constructVirtualNodeID(i);
            VirtualNode vnode(vnodeID, nodeID);
            vnodes.push_back(vnode);
        }
    }
    
    std::string getNodeID() const{ 
        return nodeID;
    }
    
    int getNumVirtualNodes() const{
        return numVirtualNodes;
    }
    
    std::vector<VirtualNode> getVirtualNodes() const{
        return vnodes;
    }
    
    
    
    
private:
    std::string constructVirtualNodeID(int i){
        return nodeID + "_" + std::to_string(i);
    }
    
    std::string nodeID;  // ID = IP address?
    int numVirtualNodes;  // for better load balancing
    std::vector<VirtualNode> vnodes;
};


class VirtualNodeHasher{
public:
    std::size_t operator()(const VirtualNode& vnode) const{
        std::string key = vnode.getVNodeID();
        return stringHasher(key);
    }
    typedef std::size_t ResultType;
private:
    std::hash<std::string> stringHasher;
};


inline std::ostream& operator<<(std::ostream& os, const StorageNode& node){
    os << node.getNodeID();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const VirtualNode& vnode){
    os << vnode.getVNodeID();
    return os;
}

#endif