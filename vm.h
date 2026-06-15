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


typedef struct
{
    ObjFunction* function;
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
    Value  stack[STACK_MAX];
    Value* stackTop;
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Obj* objects;

    Hashmap strings;
    Hashmap globals;
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



#endif //OLI_NAT_VM_H
