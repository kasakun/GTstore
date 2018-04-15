#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "consistent_hash_ring.h"
#include "gt_object.h"
#include "gt_storage_node.h"
#include "gt_manager.h"

int main(){
    StorageNode node1("node1", 4);
    StorageNode node2("node2", 4);
    StorageNode node3("node3", 4);
    StorageNode node4("node4", 4);
    
    std::unordered_map<std::string, StorageNode> nodes;  // map node name (i.e. key or ID) to node
    nodes.insert(std::pair<std::string, StorageNode>(node1.getNodeID(), node1));
    nodes.insert(std::pair<std::string, StorageNode>(node2.getNodeID(), node2));
    nodes.insert(std::pair<std::string, StorageNode>(node3.getNodeID(), node3));
    nodes.insert(std::pair<std::string, StorageNode>(node4.getNodeID(), node4));
    
    
    Manager manager(3);
    manager.addStorageNode(node1);
    manager.addStorageNode(node2);
    manager.addStorageNode(node3);
    manager.addStorageNode(node4);
    
    manager.computeStats();
    
    ObjectKeyType objectKey1 = "ywu669";
    auto preferenceList1 = manager.getPreferenceList(objectKey1);
    
    ObjectValueType objectValue1;  // a vector of strings
    objectValue1.push_back("computer science");
    objectValue1.push_back("mathematics");
    
    for(auto x : preferenceList1){
        auto nodeID = x.first;
        auto rank = x.second;
        //std::cout << "node ID = " << nodeID << ", rank = " << rank << std::endl;
        if(nodes.count(nodeID)){  // make sure nodeID exists
            nodes[nodeID].writeToVNode(rank, objectKey1, objectValue1);  // use default constructor when nodeID doesn't exist
        }
    }
    
    return 0;
}