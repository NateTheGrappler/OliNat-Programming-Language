//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_MEMORY_H
#define OLI_NAT_MEMORY_H
#endif //OLI_NAT_MEMORY_H
#include <stddef.h>
#include "Value.h"

struct Vm;
struct Obj;
//typedef struct Value Value;

#define GC_HEAP_GROW_FACTOR 2

#define GROW_CAPACITY(capacity) \
(((capacity)) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount, vm) \
    (type*)reallocate(pointer, sizeof(type) * oldCount, \
    sizeof(type) * (newCount), vm)

#define FREE_ARRAY(type, pointer, oldCount, vm) \
    reallocate(pointer, sizeof(type)*oldCount, 0, vm);

#define FREE(type, pointer, vm) reallocate(pointer, sizeof(type), 0, vm)

#define ALLOCATE(type, count, vm) \
    (type*)reallocate(NULL, 0, sizeof(type)*count, vm);

//the big old allocate function, basically just a wrapper but it's nice
//because it gives a central point for any allocation or reallocation
//of memory in the vm for the garbage collector
void* reallocate(void* pointer, size_t oldSize, size_t newSize, struct Vm* vm);
void freeObjects(struct Vm* vm);

void collectGarbage(struct Vm* vm);
void markObject(struct Obj* object, struct Vm* vm);
