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
    // Manager, to be removed
    struct sockaddr_un randomNodeAddr;
    randomNodeAddr.sun_family = AF_UNIX;
    std::string address = ROOT;
    address += env.nodeIDs.back().data();
    strcpy (randomNodeAddr.sun_path, address.data());

    ret = connect(env.clientfd, (struct sockaddr*)&randomNodeAddr, sizeof(randomNodeAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect the manager, " << strerror(errno) << std::endl;
        return false;
    }
    Packet temp;
    PacketHead tempHead = {3, 0, 0, 0, 0, 0};
    temp.head = tempHead;
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
    std::cout << "test nodes begin, coordinator " << coordinator.first << std::endl;
#endif
    //send requests to whole preference lists, temporary test
    // type, ts, seq, ack, size, rank
    PacketHead head = {2, 0, 0, 0, value.back().size(), 1};
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
    int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
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
bool Client::get(Env& env, ObjectKeyType key, ObjectValueType value){
    ssize_t ret;
    std::vector<std::pair<std::string, int>> preferenceList;
    preferenceList.push_back(std::pair<std::string, int>("node1", 2));
    // nodes
#if DEBUG
    std::cout << "test nodes begin, size " << preferenceList.size() << std::endl;
#endif
    //send requests to whole preference lists, temporary test
    // type, ts, seq, ack, size, rank
    PacketHead head = {2, 0, 0, 0, 0, 1};
    Packet p;
    p.head = head;
    memcpy(p.key, key.data(), key.size());
//    memcpy(p.value, value.data()->data(), p.head.size);

    std::vector<std::pair<std::string, int>>::iterator it;it = preferenceList.begin();
    // load nodes.
    struct sockaddr_un tempAddr;
    tempAddr.sun_family = AF_UNIX;
    std::string address = ROOT;
    address += it->first.data();
    strcpy (tempAddr.sun_path, address.data());
    // create socket
    int nodefd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (nodefd == -1) {
        std::cout<< "Client: fail to create socket with " << it->first << ", " << strerror(errno) << std::endl;
    }
    // connect
    ret = connect(nodefd, (struct sockaddr*)&tempAddr, sizeof(tempAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect " << it->first << ", " << strerror(errno) << std::endl;
        exit(0);
    }
#if DEBUG
    std::cout<<"Client: request packet to " << it->first << std::endl;
#endif
    ret = send(nodefd, &p, sizeof(p), 0);
    if (ret == -1) {
        std::cout << "Client: send error, " << strerror(errno) << std::endl;
    }

    char buf[1068];
    ret = recv(nodefd, &buf, sizeof(buf), 0);
    if (ret == -1) {
        std::cout << "Client receive ack error, " << strerror(errno) << std::endl;
    }
    if (ret == sizeof(Packet)) {
        std::cout << "Client receive ack from " << (buf + sizeof(p.head) + 20) << std::endl;
    }
    return true;
}
void Client::finalize(Env &env){}
