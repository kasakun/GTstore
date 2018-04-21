//
// Created by Zeyu Chen on 4/15/18.
// Yaohong Wu
//
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sys/un.h>
#include <sys/socket.h>
#include <zconf.h>

#include "gt_storage_node.h"

#define DEBUG 1

VirtualNode::VirtualNode(const std::string& vnodeID_, const std::string& nodeID_, const int& rank_):\
                         vnodeID(vnodeID_), nodeID(nodeID_), rank(rank_) {
    data = new std::unordered_map<ObjectKeyType, ObjectValueType>();
}

VirtualNode::~VirtualNode() {}
std::string VirtualNode::getVNodeID() const {
    return vnodeID;
}

std::string VirtualNode::getNodeID() const {
    return nodeID;
}

int VirtualNode::getRank() const {
    return rank;
}

bool VirtualNode::operator==(const VirtualNode& other) const {
    return this->vnodeID == other.vnodeID;
}

bool VirtualNode::writeKeyValuePair(ObjectKeyType key, ObjectValueType value) {
    (*data)[key] = value;
    return true;
}

bool VirtualNode::readKeyValuePair(ObjectKeyType key, ObjectValueType& value) {
    if (data->find(key) != data->end()) {
        value =  (*data)[key];
        return true;
    }
    else
        return false;
}
// Storage
StorageNode::StorageNode(const std::string& nodeID_, const int& numVirtualNodes_):\
                        nodeID(nodeID_), \
                        numVirtualNodes(numVirtualNodes_) {
    for(int i = 0; i < numVirtualNodes; ++i){
        std::string vnodeID = constructVirtualNodeID(i);
        VirtualNode vnode(vnodeID, nodeID, i);
        vnodes.push_back(vnode);
    }

    for (int i = 0; i < numVirtualNodes; i++) {
        Map vstore;
        store.push_back(vstore);
    }


}
StorageNode::~StorageNode() {}
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
    return vnodes[rank].writeKeyValuePair(key, value);
}

bool StorageNode::readVNode(int rank, ObjectKeyType& key, ObjectValueType& value) {
    if(rank < 0 || rank >= vnodes.size()) {
        std::cout << "wrong rank" << std::endl;
        return false;
    }
    return vnodes[rank].readKeyValuePair(key, value);
}

std::string StorageNode::constructVirtualNodeID(int i) {
    return nodeID + "_" + std::to_string(i);
}

Packet StorageNode::unPack(char* buf) {
    Packet p;
    memcpy(&p, buf, sizeof(Packet));
    return p;
}

void StorageNode::writeSendBack(Packet& p) {
    p.head.ack = p.head.seq + 1; // ack = seq + 1, succeeds, otherwise fails.
    p.head.seq = 0;       // sequence number of the packet
    p.head.size = 0;      // size of the value

    memcpy(p.key, nodeID.data(), sizeof(nodeID.data()));
}

void StorageNode::readSendBack(Packet& p) {
    p.head.ack = p.head.seq + 1; // ack = seq + 1, succeeds, otherwise fails.
    p.head.seq = 0;       // sequence number of the packet
}


bool StorageNode::write(Packet& p) {
    std::string keyBuf = p.key;
    std::string valueBuf = p.value;

    keyBuf.resize(20, 0);
    valueBuf.resize(p.head.size, 0);

    ObjectKeyType key = keyBuf;
    ObjectValueType value;
    value.push_back(valueBuf);
    Map map;
    std::pair<ObjectKeyType, ObjectValueType> pair (key, value);
    store[p.head.rank].insert(pair);
//    writeToVNode(p.head.rank, key, value);
//    ObjectKeyType tempk = key;
//    ObjectValueType tempv;
//    vnodes[p.head.rank].readKeyValuePair(tempk, tempv);
//    std::cout<< tempv.back().data() << std::endl;
//    return true;
    return writeToVNode(p.head.rank, key, value);
}
bool StorageNode::read(Packet& p) {
    //get key
    std::string keyBuf = p.key;
    keyBuf.resize(20, 0);
    ObjectKeyType key = keyBuf;

    if (store[p.head.rank].find(key) == store[p.head.rank].end())
        return false; // not found
    else {
        memcpy(p.value, store[p.head.rank].find(key)->second.data(),  store[p.head.rank].find(key)->second.size());
        return true;
    }
}

void StorageNode::run() {
    struct sockaddr_un nodeAddr;
    int nodefd;
    int nodeAccept;
    int ret;

    //create socket
    nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (nodefd == -1) {
        std::cout << nodeID + " fail to create socket, " << strerror(errno) << std::endl;
    }
    // set
    nodeAddr.sun_family = AF_UNIX;
    strcpy(nodeAddr.sun_path, nodeID.data());
    unlink(nodeID.data());  //clear the socket established

    // bind
    ret = bind(nodefd,  (struct sockaddr*)&nodeAddr, sizeof(nodeAddr));
    if (ret == -1) {
        std::cout <<  nodeID << " fail to bind, " << strerror(errno) << std::endl;
    }
    //listen, max queue size 10
    ret = listen(nodefd, 10);
    if (ret == -1) {
        std::cout <<  nodeID << " fail to listen, " << strerror(errno) << std::endl;
    }
    // start serving
    std::cout << nodeID << " start serving.." << std::endl;
    while(1) {
        nodeAccept = accept(nodefd, NULL, NULL);
        std::cout << nodeID << ": connected." << std::endl;
        if (nodeAccept == -1) {
            std::cout <<  nodeID << " fail to accept, " << strerror(errno) << std::endl;
        }
        ssize_t bytecount = 0;
        int opt = 1000;
        char buf[1064];
        ret = setsockopt(nodeAccept, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (ret == -1) {
            std::cout << nodeID << " fail to set socket opt, " << strerror(errno) << std::endl;
        }

        bytecount = recv(nodeAccept, buf, sizeof(buf), 0);
        if (bytecount == -1) {
            std::cout << nodeID << " fail to receive, " << strerror(errno) << std::endl;
            break;
        }
        // The packet size is Packet head + value(1024)
        if (bytecount == sizeof(Packet)) {
            // check the size of the packet
            std::cout << nodeID << " receive " << bytecount << " bytes." << std::endl;
            Packet p = unPack(buf);
            if (p.head.size != 0) {
                //write
                write(p);
                writeSendBack(p);
                bytecount = send(nodeAccept, &p, sizeof(p), 0);
                std::cout << bytecount << std::endl;
                if (bytecount == -1) {
                    std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
                    break;
                }
            } else {
                //read
                read(p);
                readSendBack(p);
                bytecount = send(nodeAccept, &p, sizeof(p), 0);
                std::cout << bytecount << std::endl;
                if (bytecount == -1) {
                    std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
                    break;
                }
            }
            
        }
    }
}

// virtual node
VirtualNodeHasher::VirtualNodeHasher() {}
VirtualNodeHasher::~VirtualNodeHasher() {}
std::size_t VirtualNodeHasher::operator()(const VirtualNode& vnode) const {
    std::string key = vnode.getVNodeID();
    return stringHasher(key);
}
