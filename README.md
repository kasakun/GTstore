# GTstore
A Distributed Key-Value System


## Defination
1. Ring - A circular dynamic list. Consistent Hashing.

2. Object - The partitioned data block.

3. (N, R, W) - Make sure W + R > N

N - Each data item is replicated at N storage nodes.

R - Minimum number of nodes that must participate in a successful read operation.

W - Minimum number of nodes that must participate in a successful write operation.


## Centralized Manager
### Ring
Consistent Hashing Table.
Range of hash function.

### Client Prerequst
Determine the object key position in the ring. Which nodes they should send their requests. Only called once per object?


## Storage Nodes
### Client Read Request
Handle read form clients.

### Client Write Request
Handle write form clients.

### Create Virtual Nodes for Data Partition
A storage node can have multiple (the number decided by its capacity) "virtual nodes" (or "token"). A virtual node is mapped to a position in the ring. Thus, a node can have multiple positions in the ring. This technique can lead to more uniform data distribution on the ring, i.e. load balancing.

### Data Replication
Know the other replication nodes?


## Client
### Pre-Request
Request centralized manager to get the right position in the ring.

### Request
Randomly choose


## Client library
### init(&env) 
initialize the client session with the manager and other control path operations. `env` stores the session info related to current client session.

### put(&env,key, value)
### get(&env,key)
### finalize(&env) 
control path cleanup operations

## Example
Shopping cart
