//
// Created by natang on 6/3/26.
//

#include <wctype.h>
#include "object.h"
#include "vm.h"

static uint32_t hashString(const char* key, int length) //basic hashing function
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];    //runs XOR (it mixes the character) with the ascii value of the character at that index in the string
        hash *= 16777619;       //the FNV prime, (another prime number with neat math properties), it spreads out the bits
    }
    return hash;
}

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

static ObjString* allocateString(char* chars, int length, uint32_t hash, struct Vm* vm)
{
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING, vm);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    //TODO: garabage collector pushes and whatnot, as well as hashtable stuff too (DONE)
    push(vm, CREATE_OBJECT_VAL((Obj*)string));
    MapSet(&vm->strings, string, CREATE_EMPTY_VAL());
    pop(vm);

    return string;
}

ObjString* copyString(const char* chars, int length, struct Vm* vm)
{
    uint32_t hash = hashString(chars, length);

    //TODO: intern the string (DONE)
    ObjString* interned = hashmapFindString(&vm->strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash, vm); //TODO: add hashing
}