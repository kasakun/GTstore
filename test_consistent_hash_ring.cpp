#include "consistent_hash_ring.h"

#include <string>
#include <iostream>

class Hasher{
public:
    std::size_t operator()(const std::string& nodeID){
        return hasher(nodeID);
    }
    typedef std::size_t ResultType;
private:
    std::hash<std::string> hasher;
};

int main(){
    ConsistentHashRing<std::string, Hasher> ring;
    
    std::string nodeID0 = "node0";
    std::string nodeID1 = "node1";
    
    ring.insert(nodeID0);
    ring.insert(nodeID1);

    return 0;
}