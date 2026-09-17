//
// Created by natang on 6/1/26.
//
#include "Value.h"

void initValueArray(ValueArray* array)
{
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

void writeValueArray(ValueArray* array, Value value, struct Vm* vm)
{
    if (array->capacity < array->count + 1)
    {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(array->capacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity, vm);
    }

    array->values[array->count] = value;
    array->count++;
}
void freeValueArray(ValueArray* array, struct Vm* vm)
{
    FREE_ARRAY(Value, array->values, array->capacity, vm);
    initValueArray(array);
}

