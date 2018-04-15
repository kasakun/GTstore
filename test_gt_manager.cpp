#include <iostream>
#include <string>


#include "consistent_hash_ring.h"
#include "gt_object.h"
#include "gt_storage_node.h"
#include "gt_manager.h"

int main(){
    StorageNode node1("node1", 3);
    StorageNode node2("node2", 6);
    Manager manager;
    manager.addStorageNode(node1);
    manager.addStorageNode(node2);
    
    manager.computeStats();
    
    return 0;
}