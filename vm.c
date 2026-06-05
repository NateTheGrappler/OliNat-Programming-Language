//
// Created by natang on 5/30/26.
//
#include "vm.h"
#include "debug.h"

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

    vm->objects = NULL;

    initMap(&vm->strings);
    initMap(&vm->globals);

}
void freeVM(Vm* vm)
{
    freeChunk(&vm->chunk);
    resetStack(vm);

    freeMap(&vm->strings);
    freeMap(&vm->globals);
    freeObjects(vm); //free all vm held objects
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
        #ifdef DEBUG_TRACE_EXECUTION
                //print the stack info for the current byte that youre on, starting from bottom of the stack to the top
                printf("Stack Info:     ");
                for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
                    printf("[ ");
                    printValue(*slot);
                    printf(" ]");
                }
                printf("\n");
                disassembleInstruction(&vm->chunk, (int)(vm->ip - vm->chunk.byteCode));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_RETURN:
                printf("ran inside of return\n");
                return INTERPRET_OK;

            //basic math operators + - * /
            case OP_ADD:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_INT_VAL(GET_INT_VAL(a) + GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_DOUBLE_VAL(da + db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,  CREATE_FLOAT_VAL(fa + fb));
                }
                break;
            }
            case OP_SUBTRACT:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_INT_VAL(GET_INT_VAL(a) - GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_DOUBLE_VAL(da - db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,  CREATE_FLOAT_VAL(fa - fb));
                }
                break;
            }
            case OP_MULTIPLY:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_INT_VAL(GET_INT_VAL(a) * GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_DOUBLE_VAL(da * db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,  CREATE_FLOAT_VAL(fa * fb));
                }
                break;
            }
            case OP_DIVIDE:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_INT_VAL(GET_INT_VAL(a) / GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_DOUBLE_VAL(da / db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,  CREATE_FLOAT_VAL(fa / fb));
                }
                break;
            }
            case OP_NEGATE:
            {
                Value constant = pop(vm);
                if (IS_FLOAT(constant))
                {
                    push(vm, CREATE_FLOAT_VAL(-GET_FLOAT_VAL(constant)));
                }
                else if (IS_DOUBLE(constant))
                {
                    push(vm, CREATE_DOUBLE_VAL(-GET_DOUBLE_VAL(constant)));
                }
                else if (IS_INT(constant))
                {
                    push(vm, CREATE_INT_VAL(-GET_INT_VAL(constant)));
                }
                break;
            }
            case OP_INVERSE:
            {
                Value constant = pop(vm);
                if (IS_BOOL(constant))
                {
                    push(vm, CREATE_BOOL_VAL(!GET_BOOL_VAL(constant)));
                }
                break;
            }
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }

            //boolean comparisions
            case OP_EQUAL:
            {
                Value a = pop(vm);
                Value b = pop(vm);

                if (IS_BOOL(a) && IS_BOOL(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_BOOL_VAL(a) == GET_BOOL_VAL(b)));
                }
                else if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_INT_VAL(a) == GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_BOOL_VAL(da == db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,   CREATE_BOOL_VAL(fa == fb));
                }
                else
                {
                    printf("Runtime Error: cannot compare values of different types you goob\n");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
                //TODO: later add strings
            }
            case OP_NOT_EQUAL:
            {
                Value a = pop(vm);
                Value b = pop(vm);

                if (IS_BOOL(a) && IS_BOOL(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_BOOL_VAL(a) != GET_BOOL_VAL(b)));
                }
                else if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_INT_VAL(a) != GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_BOOL_VAL(da != db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,   CREATE_BOOL_VAL(fa != fb));
                }
                else
                {
                    printf("Runtime Error: cannot compare values of different types you goob\n");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
                //TODO: later add strings
            }
            case OP_GREATER:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_INT_VAL(a) > GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_BOOL_VAL(da > db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,   CREATE_BOOL_VAL(fa > fb));
                }
                break;
            }
            case OP_LESS:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_INT_VAL(a) < GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_BOOL_VAL(da < db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,   CREATE_BOOL_VAL(fa < fb));
                }
                break;
            }
            case OP_GREATER_EQUAL:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_INT_VAL(a) >= GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_BOOL_VAL(da >= db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,   CREATE_BOOL_VAL(fa >= fb));
                }
                break;
            }
            case OP_LESS_EQUAL:
            {
                Value b = pop(vm);
                Value a = pop(vm);

                if (IS_INT(a) && IS_INT(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_INT_VAL(a) <= GET_INT_VAL(b)));
                }
                else if (IS_DOUBLE(a) || IS_DOUBLE(b))
                {
                    double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
                    double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
                    push(vm,  CREATE_BOOL_VAL(da <= db));
                }
                else if (IS_FLOAT(a) || IS_FLOAT(b))
                {
                    float fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (float)GET_INT_VAL(a);
                    float fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (float)GET_INT_VAL(b);
                    push(vm,   CREATE_BOOL_VAL(fa <= fb));
                }
                break;
            }

            case OP_DEFINE_GLOBAL:
            {
                //just change the top val on the stack in map then pop it off
                Value constant = READ_CONSTANT();
                ObjString* name = AS_STRING(constant);
                MapSet(&vm->globals, name, peek(vm, 0));
                pop(vm);
                break;
            }
            case OP_GET_GLOBAL:
            {
                //get the global name and toss it into the global vars map
                ObjString* name = AS_STRING(READ_CONSTANT());
                Value value;
                if (!MapGet(&vm->globals, name, &value))
                {
                    printf("Global not found: %.*s\n", name->length, name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm, value);
                break;
            }

            default:
                printf("Unknown opcode: %d\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
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

