//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_VM_H
#define OLI_NAT_VM_H

#define STACK_MAX 255

#include "common.h"
#include "ASTcompiler.h"
#include "chunk.h"

typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} vmResult;

typedef struct
{
    //the stack
    Value  stack[STACK_MAX];
    Value* stackTop;
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
