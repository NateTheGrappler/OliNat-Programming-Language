//
// Created by natang on 5/30/26.
//
#include "common.h"
#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    //if youre freeing something then yk, just free it
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    //allocate to a new point in memory with whatever new given size you got
    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}
