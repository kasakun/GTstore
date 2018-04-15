//
// Created by Zeyu Chen on 4/15/18.
//

#ifndef GTSTORE_GT_CLIENT_H
#define GTSTORE_GT_CLIENT_H

#include <unordered_map>

typedef struct sockaddr_un sockaddr;

typedef struct __Quorum {
    int n;
    int r;
    int w;
} Quorum;

typedef struct __Env {
    Quorum q;                // quorum mechanism
    sockaddr managerAddr;    // manager address
    sockaddr nodesAddr[10];  // quorum addresses
    int managerfd;
    int nodesfd[10];

    char buf[1024];          // message buf
} Env;



class Client {
public:
    Client();
    ~Client();

    bool init(Env& env);   // connect to manager
    void put(Env& env, ObjectKeyType key, ObjectValueType value);
    void get(Env& env, ObjectKeyType key, ObjectValueType value);
    void finalize(Env &env);

private:
    Env env;         //session env
    Quorum quo;      //determine the quorum mechanism
    std::unordered_map<ObjectKeyType, ObjectValueType>* data;
};


#endif //GTSTORE_GT_CLIENT_H
