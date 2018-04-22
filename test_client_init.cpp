#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <sys/socket.h>
#include <unistd.h>

#include "consistent_hash_ring.h"
#include "gt_storage_node.h"
#include "gt_manager.h"
#include "gt_client.h"


int main(){
    int pid = fork();
    
    if (pid != 0) {
        // client
        sleep(3);
        Quorum q = {3, 2, 2};
        Env session;
        // ObjectKeyType key = "21faf31r1f21faf31r1f";
        // ObjectValueType value;
        // value.push_back("{Soap, 1}, {Phone, 3}, {Wine, 6}");

        Client client(1, MANAGER, q);
        // test manager <==> client
        client.init(session);
        
        std::cout << "number of nodeIDs = " << session.nodeIDs.size() << std::endl;
        
        exit(0);
    }
    
    Manager manager(2);
    StorageNode node1("node1", 2);
    StorageNode node2("node2", 3);
    manager.addStorageNode(node1);
    manager.addStorageNode(node2);
    
    manager.run();
    
    

}