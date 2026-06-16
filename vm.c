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
    vm->hadRuntimeError = false;
    vm->frameCount = 0;
    vm->objects = NULL;

    initMap(&vm->strings);
    initMap(&vm->globals);

}
void freeVM(Vm* vm)
{
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
        runtimeError(vm, "Too many constants added onto program's stack", "STACK OVERFLOW ERROR");
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

//------------------------------------------Standard Library Stuff-----------------------------------------------///
static void defineNative(Vm* vm, const char* name, NativeFn function)
{
    //create new native, intern it's name, and toss it into globals
    ObjNative* native = newNative(function, name, strlen(name), vm);
    ObjString* nameStr = copyString(name, (int)strlen(name), vm);
    MapSet(&vm->globals, nameStr, CREATE_OBJECT_VAL((Obj*)native));
}
void registerIONatives(Vm* vm)
{
    defineNative(vm, "print", printNative);
    defineNative(vm, "println", printlnNatve);
    defineNative(vm, "intake", intakeNative);
}
void registerMathNatives(Vm* vm)
{
    defineNative(vm, "sin", sinNative);
    defineNative(vm, "cos", cosNative);
    defineNative(vm, "tan", tanNative);
    defineNative(vm, "abs", absNative);
    defineNative(vm, "ln",  lnNative);
    defineNative(vm, "log10", log10Native);
    defineNative(vm, "log2", log2Native);
    defineNative(vm, "sqrt", sqrtNative);
    defineNative(vm, "floor", floorNative);
    defineNative(vm, "ceil", ceilNative);
    defineNative(vm, "expo", expoNative);
    defineNative(vm, "pow", powNative);
}
void registerTimeNatives(Vm* vm)
{
    defineNative(vm, "clock", clockNative);
    defineNative(vm, "sleep", sleepNative);
    defineNative(vm, "time", timeNative);
    defineNative(vm, "dateString", dateStringNative);
    defineNative(vm, "timeString", timeStringNative);
}
void registerFileIONatives(Vm* vm)
{
    defineNative(vm, "readFile", readFileNative);
    defineNative(vm, "writeFile", writeFileNative);
    defineNative(vm, "appendFile", appendFileNative);
    defineNative(vm, "fileExists", fileExistsNative);
    defineNative(vm, "deleteFile", deleteFileNative);
}
void registerTypeNatives(Vm* vm)
{
    defineNative(vm, "intToStr", intToStrNative);
    defineNative(vm, "intToDouble", intToDoubleNative);
    defineNative(vm, "intToFloat", intToFloatNative);

    defineNative(vm, "doubleToStr", doubleToStrNative);
    defineNative(vm, "doubleToInt", doubleToIntNative);
    defineNative(vm, "doubleToFloat", doubleToFloatNative);

    defineNative(vm, "floatToStr", floatToStrNative);
    defineNative(vm, "floatToInt", floatToIntNative);
    defineNative(vm, "floatToDouble", floatToDoubleNative);

    defineNative(vm, "strToFloat", strToFloatNative);
    defineNative(vm, "strToInt", strToIntNative);
    defineNative(vm, "strToDouble", strToDoubleNative);
    defineNative(vm, "strToBool", strToBoolNative);

    defineNative(vm, "boolToStr", boolToStrNative);
}
void registerHashMapNatives(Vm* vm)
{

}
void registerArrayListNatives(Vm* vm)
{

}
void registerUtilsNatives(Vm* vm)
{
    defineNative(vm, "length", lengthNative);
    defineNative(vm, "assert", assertNative);
}


//-------------------------------------------_ERROR HANDLING-------------------------------------------//
void runtimeError(Vm* vm, const char* message, const char* messageType)
{
    vm->hadRuntimeError = true;
    fprintf(stderr, ":>>  %s -- ", messageType);
    fprintf(stderr, "%s\n", message);

    fprintf(stderr, "Stack trace:\n");
    for (int i = vm->frameCount - 1; i >= 0; i--)
    {
        CallFrame* frame = &vm->frames[i];
        ObjFunction* function = frame->function;

        int instruction = (int)(frame->ip - function->chunk.byteCode - 1);
        if (instruction < 0) instruction = 0;
        int line = function->chunk.lines[instruction];

        fprintf(stderr, "  [line %d] in ", line);
        if (function->nameLength == 0 || function->name == NULL)
        {
            fprintf(stderr, "<script>\n");
        }
        else
        {
            fprintf(stderr, "%.*s()\n", function->nameLength, function->name);
        }
    }
}
//----------------------------------------------VM HELPERS-------------------------------------------------//
static void concatenate(Vm* vm, Value b, Value a)
{
    ObjString* c = AS_STRING(a);
    ObjString* d = AS_STRING(b);

    int length = c->length + d->length;
    char* chars = ALLOCATE(char, length+1);
    memcpy(chars, c->chars, c->length);
    memcpy(chars + c->length, d->chars, d->length);
    chars[length] = '\0';
    ObjString* result = combineString(chars, length, vm);
    push(vm, CREATE_OBJECT_VAL((Obj*)result));
}
static bool call(ObjFunction* function, int argCount, Vm* vm)
{
    if (argCount != function->arity)
    {
        runtimeError(vm, "Function call got either more or less arguements than expected.", "INVALID CALL ERROR");
        return false;
    }
    if (vm->frameCount == FRAMES_MAX) {
        runtimeError(vm, "Too many function calls deep.", "STACK OVERFLOW ERROR");
        return false;
    }

    CallFrame* frame = &vm->frames[vm->frameCount++];
    frame->function = function;
    frame->ip = function->chunk.byteCode;
    frame->slots = vm->stackTop - argCount - 1;
    return true;
}
static bool callValue(Value callee, int argCount, Vm* vm)
{
    if (IS_OBJECT(callee))
    {
        switch (GET_OBJECT_VAL(callee)->type)
        {
            case OBJ_FUNCTION:
            {
                ObjFunction* function = (ObjFunction*)GET_OBJECT_VAL(callee);
                return call(function, argCount, vm);;
            }
            case OBJ_NATIVE:
            {
                ObjNative* native = (ObjNative*)GET_OBJECT_VAL(callee);
                Value result = native->function(argCount, vm->stackTop-argCount, vm);

                //add check for incorrect returns in native functions so runtime errors perk
                if (vm->hadRuntimeError) return false;

                vm->stackTop -= argCount + 1;
                push(vm, result);
                return true;
            }
            default:
            {
                runtimeError(vm, "Undefined calling object, how did you get here?", "SYNTAX ERROR");
                return false;
            }
        }
    }
    runtimeError(vm, "You can only call functions.", "LOGIC ERROR");
    return false;
}

//-------------------------------------------Main meat of the vm-------------------------------------------//
static vmResult run(Vm* vm)
{
    CallFrame* frame = &vm->frames[vm->frameCount - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()]) //get the stored index in the chunk buffer and then index into the chunk's stored values
#define READ_CONSTANT_LONG() (frame->function->chunk.constants.values[READ_SHORT()])
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))


    //the main meat of it all baby
    for (;;)
    {
        #ifdef DEBUG_TRACE_EXECUTION
                //print the stack info for the current byte that youre on, starting from bottom of the stack to the top
                printf("Stack Info:     ");
                int stackSize= 0;
                for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
                    printf("[ ");
                    printValue(*slot);
                    printf(" ]");
                    stackSize+=1;
                }
                printf("\n");
                printf(("Stack Size: %d"), stackSize);
                printf("\n");
                disassembleInstruction(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.byteCode));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_RETURN:
            {
                #ifdef DEBUG_TRACE_EXECUTION
                    printf("ran inside of return\n");
                #endif

                Value functionReturn = pop(vm);
                vm->frameCount--;

                if (vm->frameCount == 0)
                {
                    return INTERPRET_OK;
                }

                vm->stackTop = frame->slots;
                push(vm, functionReturn);
                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
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
                else if (IS_STRING(a) && IS_STRING(b))
                {
                    concatenate(vm, b, a);
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

                if (IS_INT(b) && GET_INT_VAL(b) == 0)
                {
                    runtimeError(vm, "Cannot logically divide a number by zero.", "DIVISON BY ZERO ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }

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
            case OP_CONSTANT_LONG:
            {
                Value constant = READ_CONSTANT_LONG();
                push(vm , constant);
                break;
            }
            case OP_POP:
            {
                pop(vm);
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
                else if (IS_STRING(a) && IS_STRING(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_OBJECT_VAL(a) == GET_OBJECT_VAL(b)));
                }
                else
                {
                    runtimeError(vm, "Cannot compare values of different types you goob", "RUNTIME ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
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
                else if (IS_STRING(a) && IS_STRING(b))
                {
                    push(vm, CREATE_BOOL_VAL(GET_OBJECT_VAL(a) != GET_OBJECT_VAL(b)));
                }
                else
                {
                    runtimeError(vm, "Cannot compare values of different types you goob", "RUNTIME ERROR");
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
                    runtimeError(vm, "Undefined Global variable.", "RUNTIME ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm, value);
                break;
            }
            case OP_SET_GLOBAL:
            {
                Value constant = READ_CONSTANT();
                ObjString* name = AS_STRING(constant);
                if (MapSet(&vm->globals, name, peek(vm, 0)))
                {
                    printf("Undefined variable '%s'\n", name->chars);
                    runtimeError(vm, "Undefined global variable.", "RUNTIME ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(vm, 0);
                break;
            }
            case OP_GET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                push(vm, frame->slots[slot]);
                break;
            }

            case OP_CREATE_ARRAY:
            {
                int arrayCount = READ_BYTE();
                ValueType arrayType = (ValueType)READ_BYTE();
                ObjStaticArray* newArray = newStaticArray(arrayCount, arrayType, vm);

                for (int i = arrayCount - 1; i >= 0; i--)
                {
                    newArray->values[i] = pop(vm);
                }
                push(vm, CREATE_OBJECT_VAL((Obj*)newArray));
                break;
            }
            case OP_SET_ARRAY_INDEX:
            {
                Value value = pop(vm);
                Value indexval = pop(vm);
                Value arrayVal = pop(vm);
                int index = GET_INT_VAL(indexval);
                ObjStaticArray* array = (ObjStaticArray*)GET_OBJECT_VAL(arrayVal);

                if (index < 0 || index >= array->length)
                {
                    runtimeError(vm, "You cannot index outside of an array's defined size", "OUT OF BOUNDS ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }
                array->values[index] = value;
                push(vm, array->values[index]);
                break;
            }
            case OP_GET_ARRAY_INDEX:
            {
                Value indexval = pop(vm);
                Value arrayVal = pop(vm);
                int index = GET_INT_VAL(indexval);

                ObjStaticArray* array = (ObjStaticArray*)GET_OBJECT_VAL(arrayVal);
                if (index < 0 || index >= array->length)
                {
                    runtimeError(vm, "You cannot index outside of an array's defined size", "OUT OF BOUNDS ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(vm, array->values[index]);
                break;
            }

            case OP_JUMP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE:
            {
                uint16_t offset = READ_SHORT();
               if (!GET_BOOL_VAL(peek(vm, 0))) frame->ip += offset;
                break;
            }
            case OP_LOOP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_CALL:
            {
                uint8_t argCount = READ_BYTE();
                Value callee = peek(vm, argCount);
                if (!IS_OBJECT(callee))
                {
                    runtimeError(vm, "You can only call instances and functions.", "LOGIC ERROR");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!callValue(callee, argCount, vm))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
            case OP_MISSING_RETURN:
            {
                runtimeError(vm, "Every single function must have a return statement!!", "MISSING RETURN ERROR");
                return INTERPRET_RUNTIME_ERROR;
            }



            default:
                runtimeError(vm, "Unknown opcode encountered.", "RUNTIME ERROR");
                return INTERPRET_RUNTIME_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_CONSTANT_LONG
}

//-----------------------------Actual VM Functionality-----------------------//
vmResult interpret(const char* source, Vm* vm)
{
    ObjFunction* topScript = compile(source, vm); //compiles into asts and then also sets up the type checker

    //error checking
    if (topScript == NULL)
    {
        return INTERPRET_COMPILE_ERROR;
    }


    //after compiling the bytecode, set up the place where its going to be read from
    push(vm, CREATE_OBJECT_VAL((Obj*)topScript));
    call(topScript, 0, vm);

    //run the code that the compiler wrote to the vms chunk
    vmResult result = run(vm);

    return result;
}

