#ifndef GT_MANAGER_H
#define GT_MANAGER_H

#include <vector>
#include <iostream>

#include "consistent_hash_ring.h"
#include "gt_storage_node.h"

class Manager{
public:
    void addStorageNode(const StorageNode& node){
        nodes.push_back(node);
        addVirturalNodes(node.getVirtualNodes());
    }
    
    void computeStats(){
        std::cout << "compute number of keys per vnode" << std::endl;
        ring.computeNumKeysPerNode();
    }
    
    
    ConsistentHashRing<VirtualNode, VirtualNodeHasher> getRing() const{
        return ring;
    }

private:
    void addVirturalNodes(const std::vector<VirtualNode>& vnodes){
        for(auto vnode : vnodes){
            ring.insert(vnode);
        }
    }

    std::vector<StorageNode> nodes;
    ConsistentHashRing<VirtualNode, VirtualNodeHasher> ring;
};

#endif