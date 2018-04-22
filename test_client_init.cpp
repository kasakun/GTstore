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
    
    if (pid == 0) {
        // client
        sleep(3);
        Quorum q = {3, 2, 2};
        Env session;
        
        ObjectKeyType key = "ywu669";
        ObjectValueType value;
        value.push_back("{Soap, 1}, {Phone, 3}, {Wine, 6}");

        Client client(1, MANAGER, q);
        
        client.init(session);
        client.put(session, key, value);

        exit(0);
    }
    
    Manager manager(2);
    StorageNode node1("node1", 2);
    StorageNode node2("node2", 3);
    manager.addStorageNode(node1);
    manager.addStorageNode(node2);
    
    
    pid = fork();
    if(pid == 0){
        pid = fork();
        if(pid == 0){
            node2.init();
            node2.run();
            exit(0);
        }
        node1.init();
        node1.run();
        exit(0);
    }
    
    manager.run();
    
}