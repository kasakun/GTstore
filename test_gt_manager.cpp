#include <iostream>
#include <string>


#include "consistent_hash_ring.h"
#include "gt_object.h"
#include "gt_storage_node.h"
#include "gt_manager.h"

int main(){
    StorageNode node1("node1", 4);
    StorageNode node2("node2", 4);
    StorageNode node3("node3", 4);
    StorageNode node4("node4", 4);
    Manager manager(3);
    manager.addStorageNode(node1);
    manager.addStorageNode(node2);
    manager.addStorageNode(node3);
    manager.addStorageNode(node4);
    
    manager.computeStats();
    
    ObjectKeyType objectKey1 = "ywu669";
    manager.getPreferenceList(objectKey1);
    
    return 0;
}