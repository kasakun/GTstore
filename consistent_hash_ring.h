#ifndef CONSISTENT_HASH_RING_H
#define CONSISTENT_HASH_RING_H

#include <map>  //std::map
#include <unordered_map>  //std::unordered_map
#include <functional>  //std::less
#include <algorithm>  //std::lower_bound
#include <utility>  //std::pair
#include <iostream>  //std::cout, std::endl, etc.
#include <limits>  // std::numeric_limits

template <class T,
          class Hasher,
          class Alloc = std::allocator<std::pair<const class Hasher::ResultType, T>>>
class ConsistentHashRing{
public:
    typedef typename Hasher::ResultType KeyType;  // typically KeyType is std::size_t
    typedef std::map<KeyType, T, std::less<KeyType>, Alloc> MapType;
    typedef class MapType::iterator iterator;
    typedef class MapType::reverse_iterator reverse_iterator;
    
    ConsistentHashRing(){
        minKey = std::numeric_limits<KeyType>::min();
        maxKey = std::numeric_limits<KeyType>::max();
        //std::cout << "min key is " << minKey << std::endl;
        //std::cout << "max key is " << maxKey << std::endl;
    }
    
    std::size_t size() const{
        return nodes.size();
    }

    bool empty() const{
        return nodes.empty();
    }

    std::pair<iterator, bool> insert(const T& node){ 
        KeyType key = hasher(node);
        //std::cout << "node = " << node << ", key = " << key << std::endl;
        return nodes.insert(std::pair<KeyType, T>(key, node));
    }

    void erase(iterator it){ 
        nodes.erase(it);
    }

    std::size_t erase(const T& node){ 
        KeyType key = hasher(node);
        return nodes.erase(key);  // return the number of elements erased
    }

    iterator find(KeyType key){ 
        if(nodes.empty()){ 
            return nodes.end();
        }
        iterator it = nodes.lower_bound(key);  // return a node whose key >= key
        if (it == nodes.end()){ 
            it = nodes.begin();
        }
        return it;
    }

    iterator begin(){ return nodes.begin(); }
    iterator end(){ return nodes.end(); }
    reverse_iterator rbegin(){ return nodes.rbegin(); }
    reverse_iterator rend(){ return nodes.rend(); }

    void computeNumKeysPerNode(){
        iterator cur = nodes.begin();
        iterator pre = nodes.end();
        pre--;
        
        while(cur != nodes.end()){
            T node = cur->second;
            KeyType diff = cur->first > pre->first ? (cur->first - pre->first) : (maxKey - pre->first + cur->first + 1);
            numKeys.insert(std::pair<T, KeyType>(node, diff));
            std::cout << "node = " << node << ", number of keys = " << diff << std::endl;
            pre = cur;
            cur++;
        }
    }
    
    
private:
    Hasher hasher;
    MapType nodes;
    
    KeyType minKey, maxKey;  // for analysis purpose
    std::unordered_map<T, KeyType, Hasher> numKeys;  // for analysis purpose
};

#endif