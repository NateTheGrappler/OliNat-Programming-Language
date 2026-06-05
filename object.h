//
// Created by natang on 6/3/26.
//

#ifndef OLI_NAT_OBJECT_H
#define OLI_NAT_OBJECT_H

#include "memory.h"
#include "common.h"
#include "Value.h"

typedef struct Vm vm;

typedef enum
{
    OBJ_STRING,
} ObjType;

//set up object class to be stored inside of value struct
typedef struct Obj
{
    ObjType type;
    struct Obj* next; //for storing in vm
} Obj;


//string representation for Oli-Nat
typedef struct ObjString
{
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
} ObjString;

static inline bool IsObjType(Value value, ObjType type)
{
    return IS_OBJECT(value) && GET_OBJECT_VAL(value)->type == type;
}

#define IS_STRING(value)       IsObjType(value, OBJ_STRING)
#define AS_STRING(value)       ((ObjString*)GET_OBJECT_VAL(value))
#define AS_CSTRING(value)      (((ObjString*)GET_OBJECT_VAL(value))->chars)

ObjString* copyString(const char* chars, int length, struct Vm* vm);

#endif //OLI_NAT_OBJECT_H
