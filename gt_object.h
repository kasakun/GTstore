#ifndef GT_OBJECT_H
#define GT_OBJECT_H

#include <string>
#include <vector>

typedef std::string ObjectKeyType;
typedef std::vector<std::string> ObjectValueType;

typedef struct __Msg {
    ObjectKeyType key;
    ObjectValueType value;
} Msg;

// current is ten elements in preference list, can be extended.
typedef struct __ManagerMsg {
    int num;
    char node[10][10];
    int value[10];
} ManagerMsg;

#endif