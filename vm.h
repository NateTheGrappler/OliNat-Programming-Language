//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_VM_H
#define OLI_NAT_VM_H

#define FRAMES_MAX 64 //recursion limit
#define STACK_MAX (256 * FRAMES_MAX)

#include "common.h"
#include "ASTcompiler.h"
#include "chunk.h"
#include "Hashmap.h"
#include "natives.h"


typedef struct
{
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} vmResult;

typedef struct Vm
{
    //the stack
    bool hadRuntimeError;
    ObjUpValue* openUpValues;
    Value  stack[STACK_MAX];
    Value* stackTop;
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Obj* objects;

    Hashmap strings;
    Hashmap globals;

    //gc stuff
    size_t bytesAllocated;
    size_t nextGC;
    int grayCapacity;
    int grayCount;
    Obj** grayStack;

} Vm;


//memory management
void initVM(Vm* vm);
void freeVM(Vm* vm);

//actual set interpret
vmResult interpret(const char* source, Vm* vm);

//stack control
void push(Vm* vm, Value value);
Value pop(Vm* vm);
Value peek(Vm* vm, int distance);

//stdlib stuff
void registerIONatives(Vm* vm);
void registerMathNatives(Vm* vm);
void registerTimeNatives(Vm* vm);
void registerFileIONatives(Vm* vm);
void registerTypeNatives(Vm* vm);
void registerHashMapNatives(Vm* vm);
void registerArrayListNatives(Vm* vm);
void registerUtilsNatives(Vm* vm);

void runtimeError(Vm* vm, const char* message, const char* messageType);


#endif //OLI_NAT_VM_H
