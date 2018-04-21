cmake_minimum_required(VERSION 3.9)
project(GTstore)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(GTstore
#        main.cpp
#        test_consistent_hash_ring.cpp
        test_client_stroage_node.cpp
        gt_storage_node.cpp
        gt_client.cpp)