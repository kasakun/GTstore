//
// Created by Zeyu Chen on 4/15/18.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <future>

#include "gt_object.h"
#include "gt_client.h"

#define MAX_THREAD 4
#define DEBUG 1
#define ROOT "/tmp/"

Client::Client() {

}
Client::Client(int id, char* addr, Quorum q) {
    clientID = id;
    managerAddr.sun_family = AF_UNIX;
    strcpy (managerAddr.sun_path, addr);
    quo = q;
}
Client::~Client() {}
//quorum, addr, addrs, fd, fds, cli_fd
void Client::init(Env& env) {
    // connect to socket
    int clientfd = socket(PF_UNIX, SOCK_STREAM, 0);
    // set env
    env = {quo, clientfd, managerAddr}; //quorum, fd, addr, addrs
    
    
    // send request to manager to get all the names of storage nodes
    getNodeInfos(env);
}

bool Client::getNodeInfos(Env& env){
    ssize_t ret;
    env.clientfd = socket(PF_UNIX, SOCK_STREAM, 0);
    ret = connect(env.clientfd, (struct sockaddr*)&env.managerAddr, sizeof(env.managerAddr));
    if(ret == -1){
        std::cout << "Client fail to connect the manager, " << strerror(errno) << std::endl;
        return false;
    }

    int requestType = 8;  // hard code
    ret = send(env.clientfd, &requestType, sizeof(requestType), 0);
    if(ret == -1){
        std::cout << "Client fail to send the key, " << strerror(errno) << std::endl;
        return false;
    }
    NodeInfoPacket nipacket;
    do{
        ret = recv(env.clientfd, &nipacket, sizeof(nipacket), 0);
        if(ret == sizeof(nipacket)){
            std::cout << "client: receive node list" << std::endl;
            for(int i = 0; i < nipacket.size; ++i){
                std::string nodeID = std::string(nipacket.nodes[i].nodeID);
                int numVNodes = nipacket.nodes[i].numVNodes;
                std::cout << "client: node ID = " << nodeID << " number of vnodes = " << numVNodes << std::endl;
                env.nodeIDs.push_back(nodeID);
            }
        }
    }while(ret == 0);
    shutdown(env.clientfd, SHUT_RDWR);    
}

bool Client::put(Env& env, ObjectKeyType key, ObjectValueType value) {
    ssize_t ret;
    std::pair<std::string, int> coordinator;
    
    struct sockaddr_un randomNodeAddr;
    randomNodeAddr.sun_family = AF_UNIX;
    std::string address = ROOT;
    address += env.nodeIDs.back().data();  // Yaohong Wu
    std::cout << "address of random node is " << address << std::endl;
    strcpy (randomNodeAddr.sun_path, address.data());
    
    env.clientfd = socket(PF_UNIX, SOCK_STREAM, 0);
    ret = connect(env.clientfd, (struct sockaddr*)&randomNodeAddr, sizeof(randomNodeAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect the random node, " << strerror(errno) << std::endl;
        return false;
    }
    Packet temp;
    
    PacketHead tempHead = {3, 0, 0, 0, 0, 0};
    temp.head = tempHead;
    std::cout << "key = " << key << " key size = " << key.size() << std::endl;
    memcpy(temp.key, key.data(), key.size());
    
    ret = send(env.clientfd, &temp, sizeof(Packet), 0);
    if (ret == -1) {
        std::cout << "Client fail to send the key, " << strerror(errno) << std::endl;
        return false;
    }
    char buf[1068];
    ret = recv(env.clientfd, &buf, sizeof(buf), 0);
    if (ret == sizeof(buf)) {
        memcpy(&temp, buf, sizeof(Packet));
        std::cout << "Client receive " << temp.value << std::endl;
        coordinator.first = temp.value;
        coordinator.second = temp.head.rank;
    }
    shutdown(env.clientfd, SHUT_RDWR);

    // nodes
#if DEBUG
    std::cout << "test nodes begin, coordinator " << coordinator.first << " rank = " << coordinator.second << std::endl;
#endif
    // send request (i.e. a key-value pair) to coordinator
    // type, ts, seq, ack, size, rank
    PacketHead head = {2, 0, 0, 0, value.back().size(), coordinator.second};  // Yaohong
    Packet p;
    p.head = head;
    memcpy(p.key, key.data(), key.size());
    memcpy(p.value, value.data()->data(), p.head.size);

    // load nodes.
    struct sockaddr_un tempAddr;
    tempAddr.sun_family = AF_UNIX;
    std::string coorAddr = ROOT;
    coorAddr += coordinator.first.data();
    strcpy (tempAddr.sun_path, coorAddr.data());
    // create socket
    int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);  // nodefd should be clientfd
    if (nodefd == -1) {
        std::cout<< "Client: fail to create socket with " << coordinator.first << ", " << strerror(errno) << std::endl;
    }
    // connect
    ret = connect(nodefd, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect " << coordinator.first << ", " << strerror(errno) << std::endl;
        exit(0);
    }
#if DEBUG
    std::cout<<"Client: send packet to " << coordinator.first << std::endl;
#endif
    ret = send(nodefd, &p, sizeof(p), 0);
    if (ret == -1) {
        std::cout << "Client: send error, " << strerror(errno) << std::endl;
    }

    ret = recv(nodefd, &buf, sizeof(buf), 0);
    if (ret == -1) {
        std::cout << "Client receive ack error, " << strerror(errno) << std::endl;
    }
    if (ret == sizeof(Packet)) {
        std::cout << "Client receive ack from " << (buf + 24) << std::endl;
    }
    return true;
}


bool Client::get(Env& env, ObjectKeyType key, ObjectValueType& value){
    ssize_t ret;
    std::pair<std::string, int> coordinator;
    
    struct sockaddr_un randomNodeAddr;
    randomNodeAddr.sun_family = AF_UNIX;
    std::string address = ROOT;
    address += env.nodeIDs.back().data();  // Yaohong Wu
    std::cout << "address of random node is " << address << std::endl;
    strcpy (randomNodeAddr.sun_path, address.data());
    
    env.clientfd = socket(PF_UNIX, SOCK_STREAM, 0);
    ret = connect(env.clientfd, (struct sockaddr*)&randomNodeAddr, sizeof(randomNodeAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect the random node, " << strerror(errno) << std::endl;
        return false;
    }
    Packet temp;
    
    PacketHead tempHead = {3, 0, 0, 0, 0, 0};
    temp.head = tempHead;
    std::cout << "key = " << key << " key size = " << key.size() << std::endl;
    memcpy(temp.key, key.data(), key.size());
    
    ret = send(env.clientfd, &temp, sizeof(Packet), 0);
    if (ret == -1) {
        std::cout << "Client fail to send the key, " << strerror(errno) << std::endl;
        return false;
    }
    char buf[1068];
    ret = recv(env.clientfd, &buf, sizeof(buf), 0);
    if (ret == sizeof(buf)) {
        memcpy(&temp, buf, sizeof(Packet));
        std::cout << "Client receive " << temp.value << std::endl;
        coordinator.first = temp.value;
        coordinator.second = temp.head.rank;
    }
    shutdown(env.clientfd, SHUT_RDWR);
    
// nodes
#if DEBUG
    std::cout << "test nodes begin, coordinator " << coordinator.first << " rank = " << coordinator.second << std::endl;
#endif
    // send a request to coordinator 
    // type, ts, seq, ack, size, rank
    PacketHead head = {2, 0, 0, 0, 0, coordinator.second};
    Packet p;
    p.head = head;
    memcpy(p.key, key.data(), key.size());

    // address of coordinator
    struct sockaddr_un tempAddr;
    tempAddr.sun_family = AF_UNIX;
    address = ROOT;
    address += coordinator.first.data();
    strcpy (tempAddr.sun_path, address.data());
    
    // create socket
    int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (nodefd == -1) {
        std::cout<< "Client: fail to create socket" << strerror(errno) << std::endl;
    }
    
    // connect
    ret = connect(nodefd, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect " << coordinator.first << ", " << strerror(errno) << std::endl;
        exit(0);
    }
    
#if DEBUG
    std::cout<<"Client: send a request packet to " << coordinator.first << std::endl;
#endif
    ret = send(nodefd, &p, sizeof(p), 0);
    
    if (ret == -1) {
        std::cout << "Client: send error, " << strerror(errno) << std::endl;
    }

    //char buf[1068];
    ret = recv(nodefd, &buf, sizeof(buf), 0);
    if (ret == -1) {
        std::cout << "Client receive ack error, " << strerror(errno) << std::endl;
    }
    if (ret == sizeof(Packet)) {
        std::cout << "Client's value = " << (buf + sizeof(p.head) + 20) << std::endl;
        char* tmp;
        memcpy(tmp, buf + sizeof(p.head) + 20, 1024);
        std::string str(tmp);
        value[0] = str;
    }
    return true;
}


void Client::finalize(Env &env){}
