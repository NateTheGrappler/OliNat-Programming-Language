//
// Created by natang on 6/4/26.
//

#ifndef OLI_NAT_HASHMAP_H
#define OLI_NAT_HASHMAP_H

#include "object.h"

//struct for each bucket inside of the hashmap
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

void initMap(Hashmap* hashmap);
void freeMap(Hashmap* hashmap);

bool MapSet(Hashmap* hashmap, ObjString* key, Value value);
bool MapGet(Hashmap* hashmap, ObjString* key, Value* value);
bool MapDelete(Hashmap* hashmap, ObjString* key);

ObjString* hashmapFindString(Hashmap* hashmap, const char* chars, int length, uint32_t hash); //for interning strings

//TODO: add garabage collector stuff here


#endif //OLI_NAT_HASHMAP_H
