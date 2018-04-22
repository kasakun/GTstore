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
//#include <zconf.h>
#include <unistd.h>
#include <future>

#include "gt_storage_node.h"


#define MAX_THREAD 4
#define DEBUG 1


/* 
 * Virtual Node
 */
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

bool StorageNode::writeToLocalVNode(int rank, ObjectKeyType key, ObjectValueType value) {
    if(rank < 0 || rank >= vnodes.size()) {
        std::cout << "wrong rank" << std::endl;
        return false;
    }
    return vnodes[rank].writeKeyValuePair(key, value);
}

bool StorageNode::readFromLocalVNode(int rank, ObjectKeyType& key, ObjectValueType& value) {
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
    //store[p.head.rank].insert(pair); // deleted -Yaohong
    writeToLocalVNode(p.head.rank, key, value);
    ObjectKeyType tempk = key;
    ObjectValueType tempv;
    vnodes[p.head.rank].readKeyValuePair(tempk, tempv);
    std::cout << nodeID << ":" << tempv.back().data() << std::endl;
    return true;
//    return writeToVNode(p.head.rank, key, value);
}


bool StorageNode::read(Packet& p) {
    //get key
    std::string keyBuf = p.key;
    keyBuf.resize(20, 0);
    ObjectKeyType key = keyBuf;
    ObjectValueType value;
    if (vnodes[p.head.rank].readKeyValuePair(key, value)) {
        p.head.size = value.back().size();
        memcpy(p.value, value.back().data(), p.head.size);
        return true;
    } else {
        return false;
    }
}

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
    std::cout << nodeID << ": connected." << std::endl;
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
        std::cout << nodeID << " receive " << bytecount << " bytes." << std::endl;
        p = unPack(buf);
        return true;
    } else {
        std::cout << strerror(errno) << std::endl;
        return false;
    }
}

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
    //
    std::vector<std::pair<std::string, int>> preferenceList;
    preferenceList.push_back(std::pair<std::string, int>("node1", 2));
    preferenceList.push_back(std::pair<std::string, int>("node2", 1));
    preferenceList.push_back(std::pair<std::string, int>("node3", 1));

    //
    ssize_t bytecount;
    if (p.head.size != 0) {
        //write to all nodes on the list
        write(p);
//        std::cout << "I read"<< p.value << std::endl;
        p.head.type = 1; // node communication
        writeToNodes(p, preferenceList);
        writeBackPack(p);

        bytecount = send(nodeAccept, &p, sizeof(p), 0);

        if (bytecount == -1) {
            std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
            return false;
        }

    } else {
        //read
        read(p);// the head.size of the packet is changed
        p.head.type = 1; // node communication
        Packet temp;
        temp.head = p.head;temp.head.size = 0;memcpy(temp.key, p.key, 20);
        readFromNodes(temp, preferenceList);
        if (p.head.timpStamp > temp.head.timpStamp) {
            readBackPack(p);
            bytecount = send(nodeAccept, &p, sizeof(p), 0);
        }

        else {
            readBackPack(temp);
            bytecount = send(nodeAccept, &p, sizeof(p), 0);
        }
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
    memcpy(p.key, pair.first.data(), sizeof(pair.first.data()));
    bytecount = send(nodeAccept, &p, sizeof(p), 0);

    if (bytecount == -1) {
        std::cout << nodeID << " fail to send ack, " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

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
        std::cout << nodeID<< " send packet to " << it->first << std::endl;
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

    for (int i = 0; i < 2; ++i) {
        foo[i] = std::async(std::launch::async, [i, &nodesfd, &counter, &mtxcounter]{
            char buf[1068];
            int ret = recv(nodesfd[i], &buf, sizeof(buf), 0);
            if (ret == -1) {
                std::cout << "receive ack error, " << strerror(errno) << std::endl;
            }
            if (ret == sizeof(Packet)) {
                std::cout << "receive ack from " << (buf + 24) << std::endl;
                mtxcounter.lock();
                ++counter;
                mtxcounter.unlock();
            }
            return ret;
        });
    }
    for (int i = 0; i < 2; ++i) {
        foo[i].get();
    }

    if (counter == 2) {
        std::cout << nodeID << " all send finish" << std::endl;
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
        ret = send(nodefd, &p, sizeof(p), 0);
        if (ret == -1) {
            std::cout << nodeID << " send error, " << strerror(errno) << std::endl;
        }
    }

    // check if num of return reach quo.w, write success, parallel
    std::future<void> foo[MAX_THREAD];
    std::mutex mtxcounter;
    std::mutex mtxpacket;
    int counter = 0;

    for (int i = 0; i < 2; ++i) {
        foo[i] = std::async(std::launch::async, [i, &p, &nodesfd, &counter, &mtxpacket, &mtxcounter]{
            char buf[1068];
            int ret = recv(nodesfd[i], &buf, sizeof(buf), 0);
            if (ret == -1) {
                std::cout << "receive error, " << strerror(errno) << std::endl;
            }
            if (ret == sizeof(Packet)) {
                std::cout << "receive value " << (buf + sizeof(p.head) + 20) << std::endl;
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
    for (int i = 0; i < 2; ++i) {
        foo[i].get();
    }

    if (counter == 2) {
        std::cout << nodeID << " all send finish" << std::endl;
        return true;
    }
    else {
        std::cout << nodeID << " send fail" << std::endl;
        return false;
    }
}

std::pair<std::string, int> StorageNode::findCoordinator(ObjectKeyType key) {
    std::hash<ObjectKeyType> hasher;
    auto hashedKey = hasher(key);
    std::cout << "hashed key = " << hashedKey << std::endl;

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
            default:
                std::cout << nodeID << ": Message type not found!" << std::endl;
                break;
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
