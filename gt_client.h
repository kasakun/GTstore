//
// Created by Zeyu Chen on 4/15/18.
//

#ifndef GTSTORE_GT_CLIENT_H
#define GTSTORE_GT_CLIENT_H

#include <sys/un.h>
#include <unordered_map>

#include "gt_object.h"


typedef struct __Quorum {
    int n;
    int r;
    int w;
} Quorum;

typedef struct __Env {
    Quorum q;                // quorum mechanism
    int clientfd;            // called by client
    struct sockaddr_un managerAddr;    // manager address
    struct sockaddr_un nodesAddr[10];  // quorum addresses
//    char buf[1024];          // message buf
} Env;



class Client {
public:
    Client();
    Client(int id, char* addr, Quorum q); // set manager addr, quorum rule
    ~Client();

    void init(Env& env);   // set the env input
    bool put(Env& env, ObjectKeyType key, ObjectValueType value);
    void get(Env& env, ObjectKeyType key, ObjectValueType value);
    void finalize(Env &env);

private:
    int id;
    int clientfd;
    struct sockaddr_un managerAddr;

    Quorum quo;      //determine the quorum mechanism
    std::unordered_map<ObjectKeyType, ObjectValueType>* data;
};


#endif //GTSTORE_GT_CLIENT_H
