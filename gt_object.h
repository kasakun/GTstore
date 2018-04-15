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

#endif