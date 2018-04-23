#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sys/un.h>
#include <sys/socket.h>
//#include <zconf.h>
#include <unistd.h>
#include <future>

#include "gt_manager.h"

#define DEBUG 0
bool Manager::createListenSocket(int& managerfd){
    struct sockaddr_un managerAddr;
    int ret;
    //create socket
    managerfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (managerfd == -1) {
        std::cout << "manager fail to create socket, " << strerror(errno) << std::endl;
        return false;
    }
    // set
    managerAddr.sun_family = AF_UNIX;
    std::string address = ROOT;
    address += this->managerID;
    strcpy(managerAddr.sun_path, address.data());
    unlink(address.data());  //clear the socket established

    // bind
    ret = bind(managerfd,  (struct sockaddr*)&managerAddr, sizeof(managerAddr));
    if (ret == -1) {
        std::cout << "manager fail to bind, " << strerror(errno) << std::endl;
        return false;
    }
    //listen, max queue size 10
    ret = listen(managerfd, 10);
    if (ret == -1) {
        std::cout <<"manager fail to listen, " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool Manager::receiveMessage(int& managerfd, int& managerAccept, char* buf, int& type){
    int ret;
    ssize_t bytecount = 0;
    int opt = 1000;  // hard code

    managerAccept = accept(managerfd, NULL, NULL);
#if DEBUG
    std::cout << "manager: connected." << std::endl;
#endif
    if (managerAccept == -1) {
        std::cout << "manager fail to accept, " << strerror(errno) << std::endl;
    }

    ret = setsockopt(managerAccept, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1) {
        std::cout << "manager fail to set socket opt, " << strerror(errno) << std::endl;
    }

    bytecount = recv(managerAccept, buf, sizeof(int), 0);
    if (bytecount == -1) {
        std::cout << "manager fail to receive, " << strerror(errno) << std::endl;
        return false;
    }
    type = *((int*)buf);
    if (bytecount == sizeof(int)) {
        // check the size of the packet
#if DEBUG
        std::cout << "manager receive " << bytecount << " bytes." << std::endl;
#endif
        return true;
    } else {
        std::cout << strerror(errno) << std::endl;
        return false;
    }
}

bool Manager::requestHandler(int managerfd, int managerAccept, int type){
    NodeInfoPacket nipack;
    memset(&nipack, 0, sizeof(nipack));
    nipack.size = nodes.size();
    for(int i = 0; i < nodes.size(); ++i){
        //nipack.nodes[i].nodeID = nodes[i].nodeID.data();
        memcpy(nipack.nodes[i].nodeID, nodes[i].nodeID.data(), nodes[i].nodeID.size());
        nipack.nodes[i].numVNodes = nodes[i].numVirtualNodes;
    }
    ssize_t bytecount;
    bytecount = send(managerAccept, &nipack, sizeof(nipack), 0);
    if(bytecount == -1){
        std::cout << "manager send fail" << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void Manager::run(){
    int managerfd;
    int managerAccept;
    if (!createListenSocket(managerfd))
        return;
    // start serving
    std::cout << "manager start serving.." << std::endl;
    while(1) {
        char* buf = new char[sizeof(int)];
        int type;
        receiveMessage(managerfd, managerAccept, buf, type);
        requestHandler(managerfd, managerAccept, type);
    }
}