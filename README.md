# GTStore
A Distributed Key-Value System


## Definition
1. Ring - A circular dynamic list, the range of consistent hashing.

2. Object - The partitioned data block. A (key, value) pair.

3. (N, R, W) - Make sure W + R > N, and W + W > N.

   N - Each data item is replicated at N storage nodes.

   R - Minimum number of nodes that must participate in a successful read operation.

   W - Minimum number of nodes that must participate in a successful write operation.


## Centralized Manager
### Ring
Consistent Hashing Table.
Range of hash function.
Should the ring be in every storage node so that a client can send its request to each storage node?

### Client Pre-requst
Determine the object key position in the ring.
Find the coordinator, i.e. the first storage node in the preference list, for an object?


## Storage Nodes
### Client Read Request
Handle read form clients.

### Client Write Request
Handle write form clients.

### Create Virtual Nodes for Data Partition
A storage node can have multiple (the number decided by its capacity) "virtual nodes" (or "token"). A virtual node is mapped to a position in the ring. Thus, a node can have multiple positions in the ring. This technique can lead to more uniform data distribution on the ring, i.e. load balancing.

### Data Replication
Determine the other replication nodes by asking the ring.


## Client
### Pre-Request
Request centralized manager (or any storage node for better performance) to get the right position in the ring.

### Request
Find the coordinator (a storage node) for an object key. Let the coordinator do the stuff!


## Client library
### init(&env)
Initialize the client session with the manager and other control path operations. `env` stores the session info related to current client session.

### put(&env,key, value)
### get(&env,key)
### finalize(&env) 
Control path cleanup operations.

## Example
Shopping cart


## List To Do

## Manager to nodes communication

## get, timestamp in database required

## init, finalized

## test file
