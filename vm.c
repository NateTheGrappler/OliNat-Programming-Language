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

void push(Vm* vm, double value)
{
    *vm->stackTop = value;
    vm->stackTop = vm->stack;
}
double pop(Vm* vm)
{
    vm->stackTop--;
    return *vm->stackTop;
}
double peek(Vm* vm, int distance)
{
    return vm->stackTop[-1 - distance];
}


//-----------------------------Actual VM Functionality-----------------------//
vmResult interpret(const char* source, Vm* vm)
{
    compile(source);

    return INTERPRET_OK;
}

