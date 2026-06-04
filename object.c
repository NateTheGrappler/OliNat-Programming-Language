//
// Created by natang on 6/3/26.
//

#include <wctype.h>
#include "object.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType, vm) \
    (type*)allocateObject(sizeof(type), objectType, vm);

static Obj* allocateObject(size_t size, ObjType type, struct Vm* vm)
{
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm->objects;
    vm->objects = object;
    return object;
}

static ObjString* allocateString(char* chars, int length, struct Vm* vm)
{
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING, vm);
    string->length = length;
    string->chars = chars;

    //TODO: garabage collector pushes and whatnot, as well as hashtable stuff too
    // push(vm, CREATE_OBJECT_VAL((Obj*)string));
    // pop(vm);

    return string;
}

ObjString* copyString(const char* chars, int length, struct Vm* vm)
{
    char* heapChars = ALLOCATE(char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, vm); //TODO: add hashing
}