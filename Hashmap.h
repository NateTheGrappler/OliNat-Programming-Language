//
// Created by natang on 6/4/26.
//

#ifndef OLI_NAT_HASHMAP_H
#define OLI_NAT_HASHMAP_H

#include "hashmap_types.h"


typedef struct Vm vm;

//struct for each bucket inside of the hashmap


void initMap(Hashmap* hashmap);
void freeMap(Hashmap* hashmap, struct Vm* vm);

bool MapSet(Hashmap* hashmap, ObjString* key, Value value, struct Vm* vm);
bool MapGet(Hashmap* hashmap, ObjString* key, Value* value);
bool MapDelete(Hashmap* hashmap, ObjString* key);

ObjString* hashmapFindString(Hashmap* hashmap, const char* chars, int length, uint32_t hash); //for interning strings

//TODO: add garabage collector stuff here


#endif //OLI_NAT_HASHMAP_H
