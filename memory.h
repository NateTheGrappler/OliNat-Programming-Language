//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_MEMORY_H
#define OLI_NAT_MEMORY_H

#endif //OLI_NAT_MEMORY_H
#include <stddef.h>

typedef struct Vm;

#define GROW_CAPACITY(capacity) \
((capacity)) < 8 ? 8 : (capacity) * 2

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * oldCount, \
    sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type)*oldCount, 0);

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type)*count);

//the big old allocate function, basically just a wrapper but it's nice
//because it gives a central point for any allocation or reallocation
//of memory in the vm for the garbage collector
void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void freeObjects(struct Vm* vm);