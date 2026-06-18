//
// Created by natang on 6/3/26.
//

#ifndef OLI_NAT_OBJECT_H
#define OLI_NAT_OBJECT_H

#define MAX_PARAMS 255

#include "memory.h"
#include "common.h"
#include "Value.h"
#include "chunk.h"

typedef struct Vm vm;

typedef enum
{
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_STATIC_ARRAY,
    OBJ_NATIVE
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


//functions
typedef struct ParamInfo
{
    ValueType type;
    const char* name;
    int length;
    bool isOptional;
} ParamInfo;

typedef struct ObjFunction
{
    Obj obj;
    int arity;
    ParamInfo params[MAX_PARAMS];
    ValueType returnType;
    Chunk chunk;
    const char* name;
    int nameLength;
} ObjFunction;
typedef struct ObjStaticArray
{
    Obj obj;
    ValueType arrayType;
    int length;
    Value* values;
} ObjStaticArray;

typedef Value (*NativeFn)(int argCount, Value* args, struct Vm* vm);
typedef struct ObjNative
{
    Obj obj;
    NativeFn function;
    const char* name;
    int nameLength;
} ObjNative;


static inline bool IsObjType(Value value, ObjType type)
{
    return IS_OBJECT(value) && GET_OBJECT_VAL(value)->type == type;
}

#define IS_STRING(value)       IsObjType(value, OBJ_STRING)
#define AS_STRING(value)       ((ObjString*)GET_OBJECT_VAL(value))
#define AS_CSTRING(value)      (((ObjString*)GET_OBJECT_VAL(value))->chars)

ObjString* copyString(const char* chars, int length, struct Vm* vm);
ObjString* combineString(char* chars, int length, struct Vm* vm);
ObjFunction* newFunction(const char* name, int nameLength, ValueType returnType, struct Vm* vm);
ObjStaticArray* newStaticArray(int count, ValueType type, struct Vm* vm);
ObjNative* newNative(NativeFn function, const char* name, int length, struct Vm* vm);
#endif //OLI_NAT_OBJECT_H
