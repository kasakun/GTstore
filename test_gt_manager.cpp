#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "consistent_hash_ring.h"
#include "gt_object.h"
#include "gt_storage_node.h"
#include "gt_manager.h"

int main(){
    Manager manager(1);
    std::string name = "node";
    int numNodes = 10;
    int numVNodes = 10;

    for(int i = 0; i < numNodes; ++i) {
        std::string nodeID = name + std::to_string(i);
        StorageNode node(nodeID, numVNodes);
        manager.addStorageNode(node);
    }
    
    manager.computeStats();

    return 0;
}