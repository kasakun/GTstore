//
// Created by Zeyu Chen on 4/15/18.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "gt_client.h"

#define DEBUG 1

Client::Client() {

}
Client::Client(int id_, char* addr, Quorum q) {
    id = id_;
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
    ManagerMsg buf;
    do {
        ret = recv(env.clientfd, &buf, sizeof(buf), 0);
        if (ret == sizeof(buf)) {
            std::cout << "Client: receive preference list from manager" << std::endl;
            //fetch
            for (int i = 0; i < buf.num; ++i) {
                std::cout << "Client receive " << buf.node[i] << " " << buf.value[i] << std::endl;
                preferenceList.push_back(std::pair<std::string, int>(buf.node[i], buf.value[i]));
            }

        }
    }while(ret == 0);

    //start node communication
    numofNodes = preferenceList.size();

    //send requests to whole preference lists
    std::string base = "store";
    std::vector<struct sockaddr_un> nodesAddr;
    std::vector<int> nodesfd;
    for (int i = 0; i < numofNodes; ++i) {
        // load nodes
        std::string tempDomain = base;
        tempDomain.append(std::to_string(i));
        struct sockaddr_un tempAddr;
        tempAddr.sun_family = AF_UNIX;
        strcpy (tempAddr.sun_path, tempDomain.data());
        nodesAddr.push_back(tempAddr);

        // connect
        int nodefd = socket();
    }

    // check if num of return reach quo.w, write success
    int counter = 0;
    do {

        counter++;
    }while(counter == quo.w);






    shutdown(env.clientfd, SHUT_RDWR);

    // go to nodes
    //

}
void Client::get(Env& env, ObjectKeyType key, ObjectValueType value){}
void Client::finalize(Env &env){}
