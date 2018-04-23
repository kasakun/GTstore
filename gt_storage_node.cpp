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

#include <unistd.h>
#include <future>

#include "gt_storage_node.h"

#define MAX_THREAD 4
#define DEBUG 0
/* 
 * Virtual Node
 */
VirtualNode::VirtualNode(const std::string& vnodeID_, const std::string& nodeID_, const int& rank_):\
                         vnodeID(vnodeID_), nodeID(nodeID_), rank(rank_) {
    data = new std::unordered_map<ObjectKeyType, ObjectValueType>();
    versions = new std::unordered_map<ObjectKeyType, ObjectVersionType>();
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

bool VirtualNode::writeKeyVersionPair(ObjectKeyType key, ObjectVersionType version) {
    (*versions)[key] = version;
    return true;
}

bool VirtualNode::readKeyVersionPair(ObjectKeyType key, ObjectVersionType& version) {
    if(versions->find(key) != versions->end()){
        version = (*versions)[key];
        return true;
    }
    else{
        return false;
    }
}

/* 
 * Storage Node
 */
StorageNode::StorageNode(const std::string& nodeID_, const int& numVirtualNodes_):\
                        nodeID(nodeID_), \
                        numVirtualNodes(numVirtualNodes_) {
    for(int i = 0; i < numVirtualNodes; ++i){
        std::string vnodeID = constructVirtualNodeID(i);
        VirtualNode vnode(vnodeID, nodeID, i);
        vnodes.push_back(vnode);
    }

    // for (int i = 0; i < numVirtualNodes; i++) {
        // Map vstore;
        // store.push_back(vstore);
    // }
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
bool StorageNode::writeToLocalVNode(int rank, ObjectKeyType key, ObjectValueType value, ObjectVersionType version) {
    if(rank < 0 || rank >= vnodes.size()) {
        std::cout << "wrong rank" << std::endl;
        return false;
    }
    return vnodes[rank].writeKeyValuePair(key, value) && vnodes[rank].writeKeyVersionPair(key, version);
}
bool StorageNode::readFromLocalVNode(int rank, ObjectKeyType key, ObjectValueType& value, ObjectVersionType& version) {
    if(rank < 0 || rank >= vnodes.size()) {
        std::cout << "wrong rank" << std::endl;
        return false;
    }
    return vnodes[rank].readKeyValuePair(key, value) && vnodes[rank].readKeyVersionPair(key, version);
}
std::string StorageNode::constructVirtualNodeID(int i) {
    return nodeID + "_" + std::to_string(i);
}
bool StorageNode::init(){
    // first send a request to the manager and receive a list of storage nodes
    // then construct the ring using the list
    ssize_t ret;
    int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un managerAddr;
    managerAddr.sun_family = AF_UNIX;
    strcpy(managerAddr.sun_path, MANAGER);
    
    ret = connect(nodefd, (struct sockaddr*)&managerAddr, sizeof(managerAddr));
    if(ret == -1){
        std::cout << "Node fail to connect the manager, " << strerror(errno) << std::endl;
        return false;
    }

    int requestType = 16;  // hard code
    ret = send(nodefd, &requestType, sizeof(requestType), 0);
    if(ret == -1){
        std::cout << "Node fail to send the request to manager, " << strerror(errno) << std::endl;
        return false;
    }
    NodeInfoPacket nipacket;
    std::vector<StorageNode> nodes;
    do{
        ret = recv(nodefd, &nipacket, sizeof(nipacket), 0);
        if(ret == sizeof(nipacket)){
#if DEBUG
            std::cout << "Node: receive node list" << std::endl;
#endif
            for(int i = 0; i < nipacket.size; ++i){
                std::string nodeID = std::string(nipacket.nodes[i].nodeID);
                int numVNodes = nipacket.nodes[i].numVNodes;
#if DEBUG
                std::cout << "Node: node ID = " << nodeID << " number of vnodes = " << numVNodes << std::endl;
#endif
                nodes.push_back(StorageNode(nodeID, numVNodes));
            }
        }
    }while(ret == 0);
    shutdown(nodefd, SHUT_RDWR);
    
    addNodesToRing(nodes);
}
void StorageNode::addNodesToRing(std::vector<StorageNode> &nodes) {
    for(auto node : nodes){
        std::vector<VirtualNode> vnodes = node.getVirtualNodes();
        for(auto vnode: vnodes){
            ring.insert(vnode);
        }
    }
}
Packet StorageNode::unPack(char* buf) {
    Packet p;
    memcpy(&p, buf, sizeof(Packet));
    return p;
}
// node server and message handler
bool StorageNode::createListenSocket(int& nodefd) {
    struct sockaddr_un nodeAddr;
    int ret;
    //create socket
    nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (nodefd == -1) {
        std::cout << nodeID + " fail to create socket, " << strerror(errno) << std::endl;
        return false;
    }
    // set
    nodeAddr.sun_family = AF_UNIX;
    std::string address = ROOT;
    address += nodeID.data();
    strcpy(nodeAddr.sun_path, address.data());
    unlink(address.data());  //clear the socket established

    // bind
    ret = bind(nodefd,  (struct sockaddr*)&nodeAddr, sizeof(nodeAddr));
    if (ret == -1) {
        std::cout <<  nodeID << " fail to bind, " << strerror(errno) << std::endl;
        return false;
    }
    //listen, max queue size 10
    ret = listen(nodefd, 10);
    if (ret == -1) {
        std::cout <<  nodeID << " fail to listen, " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}
bool StorageNode::receiveMessage(int& nodefd, int& nodeAccept, char* buf, Packet& p) {
    int ret;
    ssize_t bytecount = 0;
    int opt = 1000;

    nodeAccept = accept(nodefd, NULL, NULL);
//    std::cout << nodeID << ": connected." << std::endl;
    if (nodeAccept == -1) {
        std::cout <<  nodeID << " fail to accept, " << strerror(errno) << std::endl;
    }

    ret = setsockopt(nodeAccept, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1) {
        std::cout << nodeID << " fail to set socket opt, " << strerror(errno) << std::endl;
    }

    bytecount = recv(nodeAccept, buf, sizeof(Packet), 0);
    if (bytecount == -1) {
        std::cout << nodeID << " fail to receive, " << strerror(errno) << std::endl;
        return false;
    }
    p = unPack(buf);
    if (bytecount == sizeof(Packet)) {
        // check the size of the packet
        p = unPack(buf);
        return true;
    } else {
        std::cout << strerror(errno) << std::endl;
        return false;
    }
}
//send back pack
void StorageNode::writeBackPack(Packet& p) {
    p.head.ack = p.head.seq + 1; // ack = seq + 1, succeeds, otherwise fails.
    p.head.seq = 0;       // sequence number of the packet
    p.head.size = 0;      // size of the value

    memcpy(p.key, nodeID.data(), sizeof(nodeID.data()));
}
void StorageNode::readBackPack(Packet& p) {
    p.head.ack = p.head.seq + 1; // ack = seq + 1, succeeds, otherwise fails.
    p.head.seq = 0;       // sequence number of the packet
}
// Each node read and write
bool StorageNode::write(Packet& p) {
    std::string keyBuf = p.key;
    std::string valueBuf = p.value;

    keyBuf.resize(20, 0);
    valueBuf.resize(p.head.size, 0);

    ObjectKeyType key = keyBuf;
    ObjectValueType value = valueBuf;

    Map map;
    std::pair<ObjectKeyType, ObjectValueType> pair (key, value);
    
    writeToLocalVNode(p.head.rank, key, value, p.head.timpStamp);
    ObjectKeyType tempk = key;
    ObjectValueType tempv;
    vnodes[p.head.rank].readKeyValuePair(tempk, tempv);
    std::cout << nodeID << " writes success! Value:" << tempv.data() << std::endl;
    return true;
}
bool StorageNode::read(Packet& p) {
    //get key
    std::string keyBuf = p.key;
    keyBuf.resize(20, 0);
    ObjectKeyType key = keyBuf;
    ObjectValueType value;
    if (readFromLocalVNode(p.head.rank, key, value, p.head.timpStamp)){
        p.head.size = value.size();
        memcpy(p.value, value.data(), p.head.size);
        std::cout << nodeID << " reads success! Value:" << value.data() << std::endl;
        return true;
    } else {
        return false;
    }
}
// Four Handlers
bool StorageNode::managerHandler(int& nodefd, int& nodeAccept, Packet& p) {
    return true;
}
bool StorageNode::nodeHandler(int& nodefd, int& nodeAccept, Packet& p) {
    ssize_t bytecount;
    if (p.head.size != 0) {
        //write
        write(p);
        writeBackPack(p);
        bytecount = send(nodeAccept, &p, sizeof(p), 0);

        if (bytecount == -1) {
            std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
            return false;
        }
    } else {
        //read
        read(p);
        readBackPack(p);
        bytecount = send(nodeAccept, &p, sizeof(p), 0);
        if (bytecount == -1) {
            std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}
bool StorageNode::clientHandler(int& nodefd, int& nodeAccept, Packet& p) {
    
    std::vector<std::pair<std::string, int>> preferenceList;

    std::string key(p.key);
    key.resize(20);
    preferenceList = getPreferenceList(key, SIZE_PREFERENCE_LIST);
#if DEBUG
    std::cout << "size of preference list = " << preferenceList.size() << std::endl;
#endif
    preferenceList.erase(preferenceList.begin()); // skip the first one, i.e. the coorrdinator
    
    ssize_t bytecount;
    if (p.head.size != 0) {  // write request
        p.head.timpStamp++;  // update version, i.e. time stamp
        // first write to local 
        write(p);
        // then write to all nodes on the list
        p.head.type = 1; // node communication
        writeToNodes(p, preferenceList);
        writeBackPack(p);
        bytecount = send(nodeAccept, &p, sizeof(p), 0);

        if (bytecount == -1) {
            std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
            return false;
        }

    } else {  // read request
        // first read from local
        read(p);// the head.size of the packet is changed
        // then read from all nodes on the list
        p.head.type = 1; // node communication
        readFromNodes(p, preferenceList);
        readBackPack(p);
        bytecount = send(nodeAccept, &p, sizeof(p), 0);
        if (bytecount == -1) {
            std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}
bool StorageNode::clientCoordinator(int& nodefd, int& nodeAccept, Packet& p) {
    ssize_t bytecount;

//    writeBackPack(p);
    ObjectKeyType key = p.key;
    key.resize(20);

    std::pair<std::string, int> pair = findCoordinator(key);

    p.head.rank = pair.second;
    memcpy(p.value, pair.first.data(), sizeof(pair.first.data()));
    bytecount = send(nodeAccept, &p, sizeof(p), 0);

    if (bytecount == -1) {
        std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}
//Coordinator Functions
bool StorageNode::writeToNodes(Packet& p, std::vector<std::pair<std::string, int>> list) {
    int ret;
    std::vector<struct sockaddr_un> nodesAddr; // quorum addresses
    std::vector<int> nodesfd;
    std::vector<std::pair<std::string, int>>::iterator it;it = list.begin();
    for (; it != list.end();++it) {
        if (it->first == nodeID)
            continue;
        struct sockaddr_un tempAddr;
        tempAddr.sun_family = AF_UNIX;
        std::string address = ROOT;
        address += it->first;
        strcpy (tempAddr.sun_path, address.data());
        int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
        if (nodefd == -1) {
            std::cout << nodeID << " fail to create socket with " << it->first << ", " << strerror(errno) << std::endl;
        }
        // connect
        ret = connect(nodefd, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
        if (ret == -1) {
            std::cout << nodeID <<" fail to connect " << it->first << ", " << strerror(errno) << std::endl;
            exit(0);
        }
        nodesfd.push_back(nodefd);
        nodesAddr.push_back(tempAddr);
#if DEBUG
        std::cout << nodeID << " send packet to " << it->first << std::endl;
#endif
        nodefd = nodesfd.back();
        ret = send(nodefd, &p, sizeof(p), 0);
        if (ret == -1) {
            std::cout <<nodeID <<" send error, " << strerror(errno) << std::endl;
        }
    }

    // check if num of return reach quo.w, write success, parallel
    std::future<int> foo[MAX_THREAD];
    std::mutex mtxcounter;
    int counter = 0;

    int w = QUORUM_W;
    
    for (int i = 0; i < w; ++i) {
        foo[i] = std::async(std::launch::async, [i, &nodesfd, &counter, &mtxcounter]{
            char buf[1068];
            int ret = recv(nodesfd[i], &buf, sizeof(buf), 0);
            if (ret == -1) {
                std::cout << "receive ack error, " << strerror(errno) << std::endl;
            }
            if (ret == sizeof(Packet)) {
#if DEBUG
                std::cout << "receive ack from " << (buf + 24) << std::endl;
#endif
                mtxcounter.lock();
                ++counter;
                mtxcounter.unlock();
            }
            return ret;
        });
    }
    for (int i = 0; i < w; ++i) {
        foo[i].get();
    }

    if (counter == w) {
        std::cout << "Coordinator " << nodeID << " all " << counter << " writes finish" << std::endl;
        return true;
    }
    else {
        std::cout << nodeID << " send fail" << std::endl;
        return false;
    }
}
bool StorageNode::readFromNodes(Packet& p, std::vector<std::pair<std::string, int>> list) {
    int ret;
    std::vector<struct sockaddr_un> nodesAddr; // quorum addresses
    std::vector<int> nodesfd;
    std::vector<std::pair<std::string, int>>::iterator it;it = list.begin();

    for (; it != list.end();++it) {
        if (it->first == nodeID)
            continue;
        struct sockaddr_un tempAddr;
        tempAddr.sun_family = AF_UNIX;
        std::string address = ROOT;
        address += it->first;

        strcpy (tempAddr.sun_path, address.data());
        int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
        if (nodefd == -1) {
            std::cout << nodeID <<" fail to create socket with " << it->first << ", " << strerror(errno) << std::endl;
        }
        // connect
        ret = connect(nodefd, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
        if (ret == -1) {
            std::cout << nodeID <<" fail to connect " << it->first << ", " << strerror(errno) << std::endl;
            exit(0);
        }
        nodesfd.push_back(nodefd);
        nodesAddr.push_back(tempAddr);
#if DEBUG
        std::cout << nodeID<< " send request to " << it->first << std::endl;
#endif
        nodefd = nodesfd.back();
        Packet tempSend = p;
        tempSend.head.size = 0;
        ret = send(nodefd, &tempSend, sizeof(tempSend), 0);
        if (ret == -1) {
            std::cout << nodeID << " send error, " << strerror(errno) << std::endl;
        }
    }

    // check if num of return reach QUORUM_R, read success, parallel
    std::future<void> foo[MAX_THREAD];
    std::mutex mtxcounter;
    std::mutex mtxpacket;
    int counter = 0;

    int r = QUORUM_R;
    
    for (int i = 0; i < r; ++i) {
        foo[i] = std::async(std::launch::async, [i, &p, &nodesfd, &counter, &mtxpacket, &mtxcounter]{
            char buf[1068];
            int ret = recv(nodesfd[i], &buf, sizeof(buf), 0);
            if (ret == -1) {
                std::cout << "receive error, " << strerror(errno) << std::endl;
            }
            if (ret == sizeof(Packet)) {
#if DEBUG
                std::cout << "receive value " << (buf + sizeof(p.head) + 20) << std::endl;
#endif
                Packet temp;
                memcpy(&temp, buf, sizeof(Packet));
                mtxpacket.lock();
                if (p.head.timpStamp < temp.head.timpStamp)
                    memcpy(&p, buf, sizeof(Packet));
                mtxpacket.unlock();
                mtxcounter.lock();
                ++counter;
                mtxcounter.unlock();
            }
            return;
        });
    }
    for (int i = 0; i < r; ++i) {
        foo[i].get();
    }

    if (counter == r) {
        std::cout <<"Coordinator: "<< nodeID << " all " << counter << " reads finish" << std::endl;
        return true;
    }
    else {
        std::cout << nodeID << " send fail" << std::endl;
        return false;
    }
}
// Random selected Node
std::pair<std::string, int> StorageNode::findCoordinator(ObjectKeyType key) {
    std::hash<ObjectKeyType> hasher;
    auto hashedKey = hasher(key);
#if DEBUG
    std::cout << "key = " << key << std::endl;
    std::cout << "hashed key = " << hashedKey << std::endl;
#endif
    auto it = ring.find(hashedKey);
    std::string nodeID = (it->second).getNodeID();
    int rank = (it->second).getRank();

    return std::pair<std::string, int>(nodeID, rank);
}
void StorageNode::run() {
    int nodefd;
    int nodeAccept;
    if (!createListenSocket(nodefd)){
        std::cout << nodeID << "cannot create and listen socket" << std::endl;
        return;
    }
    // start serving
    std::cout << nodeID << " start serving.." << std::endl;
    while(1) {
        char* buf = new char[sizeof(Packet)];
        Packet p;
        int bytecount;
        receiveMessage(nodefd, nodeAccept, buf, p);
        if (bytecount == -1) {
            std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
            break;
        }
        switch (p.head.type)
        {
            case 0: // manager
                managerHandler(nodefd, nodeAccept, p);
                break;
            case 1: // node1
                nodeHandler(nodefd, nodeAccept, p);
                break;
            case 2: // client
                clientHandler(nodefd, nodeAccept, p);
                break;
            case 3: // return coorrdinator to client
                clientCoordinator(nodefd, nodeAccept, p);
                break;
            default:
                std::cout << nodeID << ": Message type not found!" << std::endl;
                break;
        }
    }

}
std::vector<std::pair<std::string, int>> StorageNode::getPreferenceList(const ObjectKeyType& objectKey, int sizeList){
    std::hash<ObjectKeyType> hasher;
    auto hashedKey = hasher(objectKey);
#if DEBUG
    std::cout << "in get plist: object key = " << objectKey << " hashed key = " << hashedKey << std::endl;
#endif
    std::vector<std::pair<std::string, int>> preferenceList;  // a list of storage nodes that should contain objectKey
    auto cur = ring.find(hashedKey);
    auto it = cur;
    int k = 0, rank;
    std::string nodeID;

    for(;k < sizeList && cur != ring.end(); ++cur){
        nodeID = (cur->second).getNodeID();

        bool skip = false;
        for(auto x : preferenceList){
            if(nodeID == x.first){
                skip = true;
                break;
            }
        }

        if(skip) continue;
        rank = (cur->second).getRank();
        preferenceList.push_back(std::pair<std::string, int>(nodeID, rank));
        k++;
#if DEBUG
        std::cout << "finding plist: virtual node key = " << (cur->first) << ", virtual node ID = " << (cur->second).getVNodeID() << std::endl;
#endif
    }

    cur = ring.begin();
    for(; k < sizeList && cur != it; ++cur){
        nodeID = (cur->second).getNodeID();

        bool skip = false;
        for(auto x : preferenceList){
            if(nodeID == x.first){
                skip = true;
                break;
            }
        }

        if(skip) continue;
        rank = (cur->second).getRank();
        preferenceList.push_back(std::pair<std::string, int>(nodeID, rank));
        k++;
#if DEBUG
        std::cout << "finding plist: virtual node key = " << (cur->first) << ", virtual node ID = " << (cur->second).getVNodeID() << std::endl;
#endif
    }

    if(k < sizeList){
        std::cout << "too few storage node!" << std::endl;
    }

#if DEBUG
    for(auto x : preferenceList){
        std::cout << "in plist: node ID = " << x.first << ", rank = " << x.second << std::endl;
    }
#endif
    return preferenceList;
};

// virtual node Hasher
VirtualNodeHasher::VirtualNodeHasher() {}
VirtualNodeHasher::~VirtualNodeHasher() {}
std::size_t VirtualNodeHasher::operator()(const VirtualNode& vnode) const {
    std::string key = vnode.getVNodeID();
    return stringHasher(key);
}
