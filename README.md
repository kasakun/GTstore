# GTstore
A Distributed Key-Value System

## Defination
1.Ring- A circular dynamic list. Consistent Hashing.
2.Object-The partitioned data block.
3.(N, R, W) - Make sure W + R > N

N- nodes containing data

R-minimum read nodes

W-minimun write nodes


## Centralized manager
### Ring
Consistent Hashing Table.

### Client Pre-requst
Determine the object key position in the ring. Which nodes they should send their requests. Only called once per object?


## Storage Nodes
### Client read request
Handle read form clients.

### Client write request
Handle write form clients.

### Replicate
Know the other replication nodes?


## Client
### Pre-Request
Request centralized manager to get the right position.

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
Shopping card


