//
// Created by Zeyu Chen on 4/15/18.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>


#include "gt_object.h"
#include "gt_client.h"

Client::Client(){}
Client::~Client() {}

bool Client::init(Env& env) {
    int managerfd = env.managerfd;
    sockaddr manager = env.managerAddr;
    int ret;
    //connect
    ret = connect(managerfd, (sockaddr*)&manager, sizeof(manager));

    if (ret == -1) {
        std::cout << "Client fail to connect the manager, " << strerror(errno) << std::endl;
        return false;
    }
    // message
    env

}

void Client::put(Env& env, ObjectKeyType key, ObjectValueType value){}
void Client::get(Env& env, ObjectKeyType key, ObjectValueType value){}
void Client::finalize(Env &env){}
