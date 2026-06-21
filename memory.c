//
// Created by natang on 5/30/26.
//
#include "common.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize, struct Vm* vm)
{
    vm->bytesAllocated += newSize - oldSize;
    if (newSize > oldSize)
    {
#ifdef DEBUG_STRESS_GC
        collectGarbage(vm);
#endif

    }

    //if youre freeing something then yk, just free it
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    //allocate to a new point in memory with whatever new given size you got
    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}


//-------freeing objects------//
static void freeObject(Obj* obj, struct Vm* vm)
{
    switch (obj->type)
    {
        case OBJ_STRING:
        {
            ObjString* string = (ObjString*)obj;
            FREE_ARRAY(char, string->chars, string->length+1, vm);
            FREE(ObjString, obj, vm);
            break;
        }
        case OBJ_FUNCTION:
        {
            ObjFunction* function = (ObjFunction*)obj;
            //FREE_ARRAY(ParamInfo, function->params, function->arity);
            freeChunk(&function->chunk, vm);
            FREE(ObjFunction, obj, vm);
            break;
        }
        case OBJ_STATIC_ARRAY:
        {
            ObjStaticArray* array = (ObjStaticArray*)obj;
            FREE_ARRAY(Value, array->values, array->length, vm);
            FREE(ObjStaticArray, array, vm);
            break;
        }
        case OBJ_CLOSURE:
        {
            ObjClosure* closure = (ObjClosure*)obj;
            FREE_ARRAY(ObjUpValue*, closure->upValues, closure->upValueCount, vm);
            FREE(ObjClosure, obj, vm);
            break;
        }
        case OBJ_UPVALUE:
        {
            FREE(ObjUpValue, obj, vm);
            break;
        }
        case OBJ_NATIVE:
        {
            FREE(ObjNative, obj, vm);
            break;
        }

    }
}
void freeObjects(struct Vm* vm)
{
    //free the list of objects stored in the vm
    Obj* object = vm->objects;
    while (object != NULL)
    {
        Obj* next = object->next;
        freeObject(object, vm);
        object=next;
    }

}

//--------Garbage collector stuff--------//
void collectGarbage(struct Vm* vm)
{
#ifdef DEBUG_LOG_GC  size_t before = vm.bytesAllocated;
    printf("-- gc begin\n");
    size_t before = vm->bytesAllocated;
#endif
}