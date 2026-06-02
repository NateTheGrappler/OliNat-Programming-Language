//
// Created by natang on 5/30/26.
//
#include "vm.h"


static void resetStack(Vm* vm)
{
    //reset the pointer of the stack to the start of array
    vm->stackTop = vm->stack;
}

void initVM(Vm* vm)
{
    resetStack(vm);
}
void freeVM(Vm* vm)
{

}

//-----------------------------Non vm stuff-----------------------//

void push(Vm* vm, Value value)
{
    //check for stack overflow
    if (vm->stackTop - vm->stack >= STACK_MAX) {
        //TODO: implement runtime errors
        return;
    }
    *vm->stackTop = value;
    vm->stackTop = vm->stackTop++;
}
Value pop(Vm* vm)
{
    vm->stackTop--;
    return *vm->stackTop;
}
Value peek(Vm* vm, int distance)
{
    return vm->stackTop[-1 - distance];
}

static vmResult run(Vm* vm)
{
#define READ_BYTE() (*vm->ip++)

    //the main meat of it all baby
    for (;;)
    {
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_ADD:
                //TODO: implement later
                break;
        }
    }

#undef READ_BYTE
}


//-----------------------------Actual VM Functionality-----------------------//
vmResult interpret(const char* source, Vm* vm)
{
    bool hadErrors = compile(source); //compiles into asts and then also sets up the type checker

    //error checking
    if (hadErrors)
    {
        return INTERPRET_COMPILE_ERROR;
    }


    //run the code that the compiler wrote to the vms chunk
    vmResult result = run(vm);

    return result;
}

