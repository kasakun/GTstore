cmake_minimum_required(VERSION 3.9)
project(GTstore)

set(CMAKE_CXX_STANDARD 11)

add_executable(GTstore
#        main.cpp
#        test_consistent_hash_ring.cpp
        test_gt_nodes.cpp
        gt_storage_node.cpp)