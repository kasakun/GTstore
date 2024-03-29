//
// Created by Zeyu Chen on 4/19/18.
//

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "consistent_hash_ring.h"
#include "gt_storage_node.h"
#include "gt_manager.h"
#include "gt_client.h"



/*
 * This test is to test communication between client lib with manager and node.
 * Current:
 * client session built
 * manager <==> client built: client first send a request to manager in put() with key, manager return preference list
 *
 * To do:
 * node <==> client
 */


int main(int argc, char** args) {
    srand(time(NULL));
    
    int pid = fork();

    if (pid == 0) {
        // client
        sleep(3);
        Quorum q = {3, 2, 2};
        Env session;
        ObjectKeyType key = "21faf31r1f21faf31r1f";
        ObjectValueType value;
        value.push_back("{Soap, 1}, {Phone, 3}, {Wine, 6}");

        Client client(1, MANAGER, q);
        // test manager <==> client
        client.init(session);
        // put
        std::cout<<"put test"<<std::endl;
        client.put(session, key, value);
        // get
        std::cout<<"get test"<<std::endl;
        client.get(session, key, value);
        exit(0);
    }
    /*
     * server: mock client-storage node
     *
     */
    // create storage node.

    //
    pid = fork();
    if (pid == 0) {
        StorageNode node1("node1", 4);
//        StorageNode node2("node2", 4);
        pid = fork();
        if (pid == 0) {
            StorageNode node2 ("node2", 4);
            pid = fork();
            if (pid == 0) {
                StorageNode node3 ("node3", 4);
                node3.run();
            }
            node2.run();
        }
        node1.run();
    }


    int serverfd;
    int serverfd2;
    int ret;
    struct sockaddr_un serverAddr;

    //create socket
    serverfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (serverfd == -1) {
        std::cout << "Server fail to socket, " << strerror(errno) << std::endl;
    }
    // set
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, MANAGER);
    unlink(MANAGER);
    // bind
    ret = bind(serverfd,  (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret == -1) {
        std::cout << "Server fail to bind, " << strerror(errno) << std::endl;
    }
    //listen
    ret = listen(serverfd, 10);
    if (ret == -1) {
        std::cout << "Server fail to listen, " << strerror(errno) << std::endl;
    }

    while(1) {
        //accept
        serverfd2 = accept(serverfd, NULL, NULL);
        if (serverfd2 == -1) {
            std::cout << "Server fail to accept, " << strerror(errno) << std::endl;
        }
        ssize_t bytecount = 0;
        int opt = 1000;
        char buf[20];
        std::cout << buf << std::endl;
        ret = setsockopt(serverfd2, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (ret == -1) {
            std::cout << "Server fail to set socket opt, " << strerror(errno) << std::endl;
        }

        bytecount = recv(serverfd2, buf,sizeof(buf), 0);
        if (bytecount == -1) {
            std::cout << "Server fail to receive, " << strerror(errno) << std::endl;
            break;
        }
        if (bytecount == 20) {
            std::cout <<"receive " << bytecount << " bytes." << std::endl;
            std::cout << buf << std::endl;
            std::vector<std::pair<std::string, int>> preferenceList;
            std::vector<std::pair<std::string, int>>::iterator it;
            // if the preference list is like:
            preferenceList.push_back(std::pair<std::string, int>("node1", 2));
//            preferenceList.push_back(std::pair<std::string, int>("node2", 1));
//            preferenceList.push_back(std::pair<std::string, int>("node3", 1));

            int i = 0;
            ManagerMsg msg;
            for (it = preferenceList.begin();it != preferenceList.end(); ++it, ++i) {
                // struct the msg, msg defined in object.h
                strcpy(msg.node[i], it->first.data());
                msg.value[i] = it->second;
            }
            msg.num = (int)preferenceList.size();

            bytecount = send(serverfd2, &msg, sizeof(msg), 0);
            if (bytecount == -1) {
                std::cout << "Server fail to send preference list, " << strerror(errno) << std::endl;
                break;
            }
            // test end break it!

        }
    }

    shutdown(serverfd,SHUT_RDWR);
    shutdown(serverfd2,SHUT_RDWR);
    return 0;
}