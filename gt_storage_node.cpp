//
// Created by Zeyu Chen on 4/15/18.
// Yaohong Wu
//
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "gt_storage_node.h"

VirtualNode::VirtualNode(const std::string& vnodeID_, const std::string& nodeID_, const int& rank_)
                        :vnodeID(vnodeID_), nodeID(nodeID_), rank(rank_){
    data = new std::unordered_map<ObjectKeyType, ObjectValueType>();
}

std::string VirtualNode::getVNodeID() const{
    return vnodeID;
}

std::string VirtualNode::getNodeID() const{
    return nodeID;
}

int VirtualNode::getRank() const{
    return rank;
}

bool VirtualNode::operator==(const VirtualNode& other) const{
    return this->vnodeID == other.vnodeID;
}

bool VirtualNode::writeKeyValuePair(ObjectKeyType key, ObjectValueType value){
    (*data)[key] = value;
    return true;
}
// Storage
StorageNode::StorageNode(const std::string& nodeID_, const int& numVirtualNodes_)\
                        :nodeID(nodeID_), \
                        numVirtualNodes(numVirtualNodes_) {
    for(int i = 0; i < numVirtualNodes; ++i){
        std::string vnodeID = constructVirtualNodeID(i);
        VirtualNode vnode(vnodeID, nodeID, i);
        vnodes.push_back(vnode);
    }
}

std::string StorageNode::getNodeID() const {
    return nodeID;
}

int StorageNode::getNumVirtualNodes() const {
    return numVirtualNodes;
}

std::vector<VirtualNode> StorageNode::getVirtualNodes() const {
    return vnodes;
}

bool StorageNode::writeToVNode(int rank, ObjectKeyType key, ObjectValueType value) {
    if(rank < 0 || rank >= vnodes.size()) {
        std::cout << "wrong rank" << std::endl;
        return false;
    }
    if(vnodes[rank].writeKeyValuePair(key, value)) return true;
    else return false;
}

std::string StorageNode::constructVirtualNodeID(int i){
    return nodeID + "_" + std::to_string(i);
}
// virtual node
VirtualNodeHasher::VirtualNodeHasher(){};
std::size_t VirtualNodeHasher::operator()(const VirtualNode& vnode) const {
    std::string key = vnode.getVNodeID();
    return stringHasher(key);
}
