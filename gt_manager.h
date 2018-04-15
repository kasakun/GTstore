#ifndef GT_MANAGER_H
#define GT_MANAGER_H

#include <vector>
#include <iostream>
#include <unordered_map>
#include <limits>

#include "consistent_hash_ring.h"
#include "gt_storage_node.h"

class Manager{
public:
    Manager(const int& numReplicas_ = 1):numReplicas(numReplicas_){}

    void addStorageNode(const StorageNode& node){
        nodes.push_back(node);
        addVirturalNodes(node.getVirtualNodes());
    }
    
    std::vector<std::string> getPreferenceList(const ObjectKeyType& objectKey){
        std::hash<ObjectKeyType> hasher;
        auto hashedKey = hasher(objectKey);
        std::cout << "hashed key = " << hashedKey << std::endl;
        
        std::vector<std::string> preferenceList;  // a list of storage nodes that should contain objectKey
        auto it = ring.find(hashedKey);
        std::string nodeID = (it->second).getNodeID();
        preferenceList.push_back(nodeID);
        int k = 1;
        std::cout << "key = " << (it->first) << ", virtual node ID = " << (it->second).getVNodeID() << std::endl;
        
        auto cur = it;
        cur++;
                
        for(;k < numReplicas && cur != ring.end(); ++cur){
            nodeID = (cur->second).getNodeID();
            
            bool skip = false;
            for(auto id : preferenceList){
                if(nodeID == id){
                    skip = true;
                    break;
                }
            }
            
            if(skip) continue;
            preferenceList.push_back(nodeID);
            k++;
            std::cout << "key = " << (cur->first) << ", virtual node ID = " << (cur->second).getVNodeID() << std::endl;
        }
        
        cur = ring.begin();
        for(; k < numReplicas && cur != it; ++cur){
            nodeID = (cur->second).getNodeID();
            
            bool skip = false;
            for(auto id : preferenceList){
                if(nodeID == id){
                    skip = true;
                    break;
                }
            }
            
            if(skip) continue;
            preferenceList.push_back(nodeID);
            k++;
            std::cout << "key = " << (cur->first) << ", virtual node ID = " << (cur->second).getVNodeID() << std::endl;
        }
        
        if(k < numReplicas){
            std::cout << "too few storage node!" << std::endl;
        }
        
        for(auto id : preferenceList){
            std::cout << "node ID = " << id << std::endl;
        }
        
        return preferenceList;
    }
    
    
    void computeStats(){
        std::cout << "compute number of keys per virtual node" << std::endl;
        ring.computeNumKeysPerNode();
        
        std::cout << "\ncompute number of keys per storage node" << std::endl;
        computeNumKeyPerStorageNode();
    }
    

private:
    void addVirturalNodes(const std::vector<VirtualNode>& vnodes){
        for(auto vnode : vnodes){
            ring.insert(vnode);
        }
    }
    
    void computeNumKeyPerStorageNode(){
        ConsistentHashRing<VirtualNode, VirtualNodeHasher>::iterator cur = ring.begin();
        ConsistentHashRing<VirtualNode, VirtualNodeHasher>::iterator pre = ring.end();
        pre--;
        
        //std::cout << "max unsigned long = " << std::numeric_limits<unsigned long>::max() << std::endl;
        unsigned long diff = std::numeric_limits<unsigned long>::max() - pre->first + cur->first + 1;
        numKeys[cur->second.getNodeID()] += diff;
        
        pre = cur;
        while(++cur != ring.end()){
            diff = cur->first - pre->first;
            numKeys[cur->second.getNodeID()] += diff;
        }
        
        for(auto n : numKeys){
            std::cout << "node ID = " << n.first << ", number of keys = " << n.second << std::endl;
        }
    }
    
    
    std::vector<StorageNode> nodes;
    ConsistentHashRing<VirtualNode, VirtualNodeHasher> ring;
    
    std::unordered_map<std::string, unsigned long> numKeys;
    
    int numReplicas;  // number of replicas per item
};

#endif