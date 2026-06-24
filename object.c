//
// Created by natang on 6/3/26.
//

#include <wctype.h>
#include "object.h"
#include "vm.h"

static uint32_t hashString(const char* key, int length) //basic hashing function
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];    //runs XOR (it mixes the character) with the ascii value of the character at that index in the string
        hash *= 16777619;       //the FNV prime, (another prime number with neat math properties), it spreads out the bits
    }
    return hash;
}

#define ALLOCATE_OBJ(type, objectType, vm) \
    (type*)allocateObject(sizeof(type), objectType, vm);

static Obj* allocateObject(size_t size, ObjType type, struct Vm* vm)
{
    Obj* object = (Obj*)reallocate(NULL, 0, size, vm);
    object->type = type;
    object->isMarked = false;

    object->next = vm->objects;
    vm->objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    return object;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash, struct Vm* vm)
{
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING, vm);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(vm, CREATE_OBJECT_VAL((Obj*)string));
    MapSet(&vm->strings, string, CREATE_EMPTY_VAL(), vm);
    pop(vm);

    return string;
}

ObjString* copyString(const char* chars, int length, struct Vm* vm)
{
    uint32_t hash = hashString(chars, length);

    //TODO: intern the string (DONE)
    ObjString* interned = hashmapFindString(&vm->strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length+1, vm);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash, vm); //TODO: add hashing
}


ObjString* combineString(char* chars, int length, Vm* vm)
{
    uint32_t hash = hashString(chars, length); //rehash a new code for the concatenated string

    ObjString* interned = hashmapFindString(&vm->strings, chars, length, hash);
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length+1,vm);
        return interned;
    }
    return allocateString(chars, length, hash, vm);
}


ObjFunction* newFunction(const char* name, int nameLength, ValueType returnType, struct Vm* vm)
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION, vm);
    function->arity = 0;
    function->name = name;
    function->nameLength = nameLength;
    function->upValueCount = 0;
    initChunk(&function->chunk);
    function->returnType = returnType;
    return function;
}
ObjStaticArray* newStaticArray(int count, ValueType type, struct Vm* vm)
{
    ObjStaticArray* array = ALLOCATE_OBJ(ObjStaticArray, OBJ_STATIC_ARRAY, vm);
    array->length = count;
    array->arrayType  =type;
    array->values = ALLOCATE(Value, count, vm);
    return array;
}
ObjNative* newNative(NativeFn function, const char* name, int length, struct Vm* vm)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE, vm);
    native->function = function;
     native->name = name;
    native->nameLength = length;
    return native;
}

ObjUpValue* newUpValue(Value* slot, struct Vm* vm)
{
    ObjUpValue* upval = ALLOCATE_OBJ(ObjUpValue, OBJ_UPVALUE, vm);
    upval->location = slot;
    upval->next = NULL;
    upval->closed = CREATE_EMPTY_VAL();
    return upval;
}
ObjClosure* newClosure(ObjFunction* function, struct Vm* vm)
{

    ObjUpValue** upvals = ALLOCATE(ObjUpValue*, function->upValueCount, vm);
    for (int i = 0; i < function->upValueCount; i++)
    {
        upvals[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE, vm);
    closure->function = function;
    closure->upValueCount = function->upValueCount;
    closure->upValues = upvals;

    return closure;
}

ObjClass* newClass(const char* name, int nameLength, struct Vm* vm)
{
    //different name for porting to another language like c++ or something
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS, vm);
    klass->name = name;
    klass->nameLength = nameLength;
    return klass;
}
ObjInstance* newInstance(ObjClass* klass,  struct Vm* vm)
{
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE, vm);
    instance->class = klass;
    return instance;
}

