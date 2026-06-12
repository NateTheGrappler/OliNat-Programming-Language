//
// Created by natang on 5/30/26.
//
#include "common.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize)
{
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
static void freeObject(Obj* obj)
{
    switch (obj->type)
    {
        case OBJ_STRING:
        {
            ObjString* string = (ObjString*)obj;
            FREE_ARRAY(char, string->chars, string->length+1);
            FREE(ObjString, obj);
            break;
        }
        case OBJ_FUNCTION:
        {
            ObjFunction* function = (ObjFunction*)obj;
            //FREE_ARRAY(ParamInfo, function->params, function->arity);
            freeChunk(&function->chunk);
            FREE(ObjFunction, obj);
            break;
        }
        case OBJ_STATIC_ARRAY:
        {
            ObjStaticArray* array = (ObjStaticArray*)obj;
            FREE_ARRAY(Value, array->values, array->length);
            FREE(ObjStaticArray, array);
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
        freeObject(object);
        object=next;
    }

}