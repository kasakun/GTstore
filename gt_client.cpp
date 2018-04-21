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
}

bool Client::put(Env& env, ObjectKeyType key, ObjectValueType value) {
    ssize_t ret;
    int numofNodes;

    std::vector<std::pair<std::string, int>> preferenceList;
    /*
     * Connect manager first
     */
    ret = connect(env.clientfd, (struct sockaddr*)&env.managerAddr, sizeof(env.managerAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect the manager, " << strerror(errno) << std::endl;
        return false;
    }

    /*
     * Prepare message and send
     */
    ret = send(env.clientfd, key.c_str(), key.length(), 0);

    if (ret == -1) {
        std::cout << "Client fail to send the key, " << strerror(errno) << std::endl;
        return false;
    }

    // wait until receive
    ManagerMsg msg;
    do {
        ret = recv(env.clientfd, &msg, sizeof(msg), 0);
        if (ret == sizeof(msg)) {
            std::cout << "Client: receive preference list from manager" << std::endl;
            //fetch
            for (int i = 0; i < msg.num; ++i) {
                std::cout << "Client receive " << msg.node[i] << " " << msg.value[i] << std::endl;
                preferenceList.push_back(std::pair<std::string, int>(msg.node[i], msg.value[i]));
            }

        }
    }while(ret == 0);

    shutdown(env.clientfd, SHUT_RDWR);

    // node communication
    numofNodes = preferenceList.size();
#if DEBUG
    std::cout << "test nodes begin, size " << preferenceList.size() << std::endl;
#endif
    //send requests to whole preference lists, temporary test
    // ts, seq, ack, size, rank
    PacketHead head = {0, 0, 0, value.back().size(), 1};
    Packet p;
    p.head = head;
    memcpy(p.key, key.data(), key.size());
    memcpy(p.value, value.data()->data(), p.head.size);

    std::vector<std::pair<std::string, int>>::iterator it;it = preferenceList.begin();
    for (int i = 0; i < numofNodes; ++i, ++it) {
        // load nodes.
        struct sockaddr_un tempAddr;
        tempAddr.sun_family = AF_UNIX;
        strcpy (tempAddr.sun_path, it->first.data());
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
        env.nodesfd.push_back(nodefd);
        env.nodesAddr.push_back(tempAddr);
#if DEBUG
        std::cout<<"Client: send packet to " << it->first << std::endl;
#endif
        nodefd = env.nodesfd.back();
        ret = send(nodefd, &p, sizeof(p), 0);
        if (ret == -1) {
            std::cout << "Client: send error, " << strerror(errno) << std::endl;
        }
    }

    // check if num of return reach quo.w, write success, parallel
    std::future<void> foo[MAX_THREAD];
    std::mutex mtxcounter;
    int counter = 0;

    for (int i = 0; i < quo.w; ++i) {
        foo[i] = std::async(std::launch::async, [i, &env, &counter, &mtxcounter]{
            char buf[1064];
            int ret = recv(env.nodesfd[i], &buf, sizeof(buf), 0);
            if (ret == -1) {
                std::cout << "Client receive ack error, " << strerror(errno) << std::endl;
            }
            if (ret == sizeof(Packet)) {
                std::cout << "Client receive ack from " << (buf + 20) << std::endl;
                mtxcounter.lock();
                ++counter;
                mtxcounter.unlock();
            }
            return;
        });
    }
    for (int i = 0; i < quo.w; ++i) {
        foo[i].get();
    }

    if (counter == quo.w) {
        std::cout<<"Client: all send finish" << std::endl;
        return true;
    }
    else {
        std::cout<<"Client: send fail" << std::endl;
        return false;
    }
}
bool Client::get(Env& env, ObjectKeyType key, ObjectValueType value){
    ssize_t ret;
    int numofNodes;

    std::vector<std::pair<std::string, int>> preferenceList;
    /*
     * Connect manager first
     */
    ret = connect(env.clientfd, (struct sockaddr*)&env.managerAddr, sizeof(env.managerAddr));
    if (ret == -1) {
        std::cout << "Client fail to connect the manager, " << strerror(errno) << std::endl;
        return false;
    }

    /*
     * Prepare message and send
     */
    ret = send(env.clientfd, key.c_str(), key.length(), 0);

    if (ret == -1) {
        std::cout << "Client fail to send the key, " << strerror(errno) << std::endl;
        return false;
    }

    // wait until receive
    ManagerMsg msg;
    do {
        ret = recv(env.clientfd, &msg, sizeof(msg), 0);
        if (ret == sizeof(msg)) {
            std::cout << "Client: receive preference list from manager" << std::endl;
            //fetch
            for (int i = 0; i < msg.num; ++i) {
                std::cout << "Client receive " << msg.node[i] << " " << msg.value[i] << std::endl;
                preferenceList.push_back(std::pair<std::string, int>(msg.node[i], msg.value[i]));
            }

        }
    }while(ret == 0);

    shutdown(env.clientfd, SHUT_RDWR);

    // node communication
    numofNodes = preferenceList.size();
#if DEBUG
    std::cout << "test nodes begin, size " << preferenceList.size() << std::endl;
#endif
    //send requests to whole preference lists, temporary test
    // ts, seq, ack, size, rank
    PacketHead head = {0, 0, 0, 0, 1};
    Packet p;
    p.head = head;
    memcpy(p.key, key.data(), key.size());
    memcpy(p.value, value.data()->data(), p.head.size);

    std::vector<std::pair<std::string, int>>::iterator it;it = preferenceList.begin();
    for (int i = 0; i < numofNodes; ++i, ++it) {
        // load nodes.
        struct sockaddr_un tempAddr;
        tempAddr.sun_family = AF_UNIX;
        strcpy (tempAddr.sun_path, it->first.data());
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
        env.nodesfd.push_back(nodefd);
        env.nodesAddr.push_back(tempAddr);
#if DEBUG
        std::cout<<"Client: send packet to " << it->first << std::endl;
#endif
        nodefd = env.nodesfd.back();
        ret = send(nodefd, &p, sizeof(p), 0);
        if (ret == -1) {
            std::cout << "Client: send error, " << strerror(errno) << std::endl;
        }
    }

    // check if num of return reach quo.w, write success, parallel
    std::future<void> foo[MAX_THREAD];
    std::mutex mtxcounter;
    int counter = 0;

    for (int i = 0; i < quo.w; ++i) {
        foo[i] = std::async(std::launch::async, [i, &env, &counter, &mtxcounter]{
            char buf[1064];
            int ret = recv(env.nodesfd[i], &buf, sizeof(buf), 0);
            if (ret == -1) {
                std::cout << "Client receive ack error, " << strerror(errno) << std::endl;
            }
            if (ret == sizeof(Packet)) {
                std::cout << "Client receive ack from " << (buf + 20) << std::endl;
                mtxcounter.lock();
                ++counter;
                mtxcounter.unlock();
            }
            return;
        });
    }
    for (int i = 0; i < quo.w; ++i) {
        foo[i].get();
    }

    if (counter == quo.w) {
        std::cout<<"Client: all send finish" << std::endl;
        return true;
    }
    else {
        std::cout<<"Client: send fail" << std::endl;
        return false;
    }
}
void Client::finalize(Env &env){}
