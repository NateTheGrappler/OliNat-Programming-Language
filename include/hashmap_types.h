//
// Created by natang on 6/25/26.
//

#ifndef OLI_NAT_HASHMAP_TYPES_H
#define OLI_NAT_HASHMAP_TYPES_H

typedef struct ObjString ObjString;
#include "Value.h"

typedef struct
{
    ObjString* key;
    Value value;
} Bucket;

typedef struct
{
    int count;
    int capacity;
    Bucket* buckets;
} Hashmap;

#endif //OLI_NAT_HASHMAP_TYPES_H
