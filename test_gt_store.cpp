#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <sys/socket.h>
#include <unistd.h>

#include "gt_object.h"
#include "consistent_hash_ring.h"
#include "gt_storage_node.h"
#include "gt_manager.h"
#include "gt_client.h"


int main(){

    std::cout << "Welcome to key-value GTStore!" << std::endl;
    int pid = fork();
    if (pid != 0) {
        // client
        sleep(3);
        Quorum q = {QUORUM_N, QUORUM_R, QUORUM_W};
        Env session;
        
        ObjectKeyType key = "ywu669zchen606OQ5780";
        ObjectValueType value = "{Soap, 1}, {Phone, 3}, {Wine, 6}";
        std::cout << "Key value is " << key << ", value is " << value.data() << std::endl;
        Client client(1, MANAGER, q);
        client.init(session);
        
        std::cout << "Test put() " << std::endl;
        client.put(session, key, value);
        std::cout << "Before get(), version of key = " << (*client.versions)[key] << std::endl;

        std::cout << "Test get() " << std::endl;
        sleep(2);
        ObjectValueType newValue;
        client.get(session, key, newValue);
        std::cout << "After get(), value = " << newValue << " version = " << (*client.versions)[key] << std::endl;
        
        client.finalize(session);
        
        exit(0);
    }
    
    Manager manager(QUORUM_N);
    StorageNode node1("node1", 4);
    StorageNode node2("node2", 4);
    StorageNode node3("node3", 4);
    StorageNode node4("node4", 4);
    manager.addStorageNode(node1);
    manager.addStorageNode(node2);
    manager.addStorageNode(node3);
    manager.addStorageNode(node4);
    
    
    pid = fork();
    if(pid == 0){
        pid = fork();
        if(pid == 0){
            pid = fork();
            if(pid == 0){
                pid = fork();
                if(pid == 0){
                    node4.init();
                    node4.run();
                    exit(0);
                }
                node3.init();
                node3.run();
                exit(0);
            }
            node2.init();
            node2.run();
            exit(0);
        }
        node1.init();
        node1.run();
        exit(0);
    }
    
    manager.run();
    exit(0);
}