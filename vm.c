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
    initChunk(&vm->chunk);
    vm->ip = vm->chunk.byteCode;

}
void freeVM(Vm* vm)
{
    freeChunk(&vm->chunk);
    resetStack(vm);
}

//-----------------------------------------------Stack stuff-------------------------------------------------------//

void push(Vm* vm, Value value)
{
    //check for stack overflow
    if (vm->stackTop - vm->stack >= STACK_MAX) {
        //TODO: implement runtime errors
        return;
    }
    *vm->stackTop = value;
    vm->stackTop++;
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

//----------------------------------------------VM HELPERS-------------------------------------------------//

//-------------------------------------------Main meat of the vm-------------------------------------------//

static vmResult run(Vm* vm)
{
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk.constants.values[READ_BYTE()]) //get the stored index in the chunk buffer and then index into the chunk's stored values

    //the main meat of it all baby
    for (;;)
    {
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_RETURN:
                printf("ran inside of return\n");
                return INTERPRET_OK;

            //basic math operators + - * /
            case OP_ADD:
                //TODO: implement later
                printf("ran inside of add\n");
                break;
            case OP_SUBTRACT:
                //TODO: implement later
                break;
            case OP_MULTIPLY:
                //TODO: implement later
                break;
            case OP_DIVIDE:
                //TODO: implement later
                break;
            case OP_CONSTANT:
                printf("adding constant\n");
                break;
        }
    }

#undef READ_BYTE
}


//-----------------------------Actual VM Functionality-----------------------//
vmResult interpret(const char* source, Vm* vm)
{
    bool hadErrors = compile(source, vm); //compiles into asts and then also sets up the type checker

    //error checking
    if (hadErrors)
    {
        return INTERPRET_COMPILE_ERROR;
    }


    //after compiling the bytecode, set up the place where its going to be read from
    vm->ip = vm->chunk.byteCode;

    //run the code that the compiler wrote to the vms chunk
    vmResult result = run(vm);

    return result;
}

