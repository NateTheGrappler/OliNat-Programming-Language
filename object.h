//
// Created by natang on 6/3/26.
//

#ifndef OLI_NAT_OBJECT_H
#define OLI_NAT_OBJECT_H

#define MAX_PARAMS 255
#define MAX_FIELDS 256

#include "memory.h"
#include "common.h"
#include "Value.h"
#include "chunk.h"
#include "Hashmap.h"


typedef struct Vm vm;
typedef struct Hashmap hashmap;

typedef enum
{
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_STATIC_ARRAY,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_CLASS,
    OBJ_INSTANCE
} ObjType;

//set up object class to be stored inside of value struct
typedef struct Obj
{
    ObjType type;
    bool isMarked;
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


//functions and their extensions
typedef struct ParamInfo
{
    ValueType type;
    const char* name;
    int length;
    bool isOptional;
} ParamInfo;

//functions
typedef struct ObjFunction
{
    Obj obj;
    int arity;
    ParamInfo params[MAX_PARAMS];
    ValueType returnType;
    Chunk chunk;
    const char* name;
    int nameLength;
    int upValueCount;
} ObjFunction;
typedef struct ObjUpValue
{
    Obj obj;
    Value* location;
    struct ObjUpValue* next;
    Value closed;
} ObjUpValue;
typedef struct ObjClosure
{
    Obj obj;
    ObjFunction* function;
    ObjUpValue** upValues;
    int upValueCount;
} ObjClosure;

//classes
typedef struct
{
    ValueType type;
    ObjString* name;
    int length;
    Value defaultValue;
} FieldInfo;
typedef struct ObjClass
{
    //basic stuff
    Obj obj;
    const char* name;
    int nameLength;

    //inner class fields
    FieldInfo fields[MAX_FIELDS];
    int fieldCount;

    Hashmap methods;

} ObjClass;
typedef struct ObjInstance
{
    Obj obj;
    ObjClass* class;
    Value* fields; //a somewhat static field since instances cannot create new fields
    int fieldCount;
} ObjInstance;


//static array literals
typedef struct ObjStaticArray
{
    Obj obj;
    ValueType arrayType;
    int length;
    Value* values;
} ObjStaticArray;

//native functions for stdlib
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
ObjUpValue* newUpValue(Value* slot, struct Vm* vm);
ObjClosure* newClosure(ObjFunction* function, struct Vm* vm);
ObjClass* newClass(const char* name, int nameLength, struct Vm* vm);
ObjInstance* newInstance(ObjClass* klass,  struct Vm* vm);
#endif //OLI_NAT_OBJECT_H
