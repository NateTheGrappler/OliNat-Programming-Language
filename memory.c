//
// Created by natang on 5/30/26.
//
#include "common.h"
#include "memory.h"
#include "vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize, struct Vm* vm)
{
    vm->bytesAllocated += newSize - oldSize;
    if (newSize > oldSize)
    {
#ifdef DEBUG_STRESS_GC
        collectGarbage(vm);
#endif

        if (vm->bytesAllocated > vm->nextGC)
        {
            collectGarbage(vm);
        }
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
        case OBJ_CLASS:
        {
            ObjClass* klass = (ObjClass*)obj;
            FREE(ObjClass, obj, vm);
            break;
        }
        case OBJ_INSTANCE:
        {
            ObjInstance* instance = (ObjInstance*)obj;
            FREE(ObjInstance, obj, vm);
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

//basic marking

void markObject(Obj* object, struct Vm* vm)
{
    if (object == NULL) return;
    if (object->isMarked) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    printValue(CREATE_OBJECT_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    if (vm->grayCapacity < vm->grayCount + 1)
    {
        vm->grayCapacity = GROW_CAPACITY(vm->grayCapacity);
        vm->grayStack = (Obj**)realloc(vm->grayStack, sizeof(Obj*) * vm->grayCapacity);
        if (vm->grayStack == NULL) exit(1); //if no space to allocate, bail program
    }
    vm->grayStack[vm->grayCount++] = object;

}
void markValue(Value value, struct Vm* vm)
{
    if (IS_OBJECT(value)) markObject(GET_OBJECT_VAL(value), vm);
}
void markArray(ValueArray* array, struct Vm* vm)
{
    for (int i = 0; i < array->count; i++)
    {
        markValue(array->values[i], vm);
    }
}
void markHashmap(Hashmap* hashmap, struct Vm* vm)
{
    for (int i = 0; i < hashmap->capacity; i++)
    {
        Bucket* bucket = &hashmap->buckets[i];
        markObject((Obj*)bucket->key, vm);
        markValue(bucket->value, vm);
    }
}
static void markRoots(struct Vm* vm)
{
    //go down stack and mark all of those as safe
    for (Value* slot = vm->stack; slot < vm->stackTop; slot++)
    {
        markValue(*slot, vm);
    }

    //do closures
    for (int i = 0; i < vm->frameCount; i++)
    {
        markObject((Obj*)vm->frames[i].closure, vm);
    }

    //get the upvalues still stored in vm
    for (ObjUpValue* upVal = vm->openUpValues; upVal != NULL; upVal = upVal->next)
    {
        markObject((Obj*)upVal, vm);
    }

    //mark all global functions
    markHashmap(&vm->globals, vm);
}

//pass through inner refrences
static void blackenObject(Obj* object, struct Vm* vm)
{
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    printValue(CREATE_OBJECT_VAL(object));
    printf("\n");
#endif

    switch (object->type)
    {
        case OBJ_CLOSURE:
        {
            ObjClosure* closure = (ObjClosure*)object;
            markObject((Obj*)closure->function, vm);
            for (int i = 0; i < closure->upValueCount; i++)
            {
                markObject((Obj*)closure->upValues[i], vm);
            }
            break;
        }
        case OBJ_FUNCTION:
        {
            //dont clear the param info since it is just a static metadata array
            ObjFunction* function = (ObjFunction*)object;
            markArray(&function->chunk.constants, vm);
            break;
        }
        case OBJ_STATIC_ARRAY:
        {
            ObjStaticArray* array = (ObjStaticArray*)object;
            for (int i = 0; i < array->length; i++)
            {
                markValue(array->values[i], vm);
            }
            break;
        }
        case OBJ_UPVALUE:
        {
            markValue(((ObjUpValue*)object)->closed, vm); //mark value internally held in upvalue
            break;
        }
        case OBJ_STRING:
        case OBJ_NATIVE:
            break;


    }
}
static void traceRefrences(struct Vm* vm)
{
    while (vm->grayCount > 0)
    {
        Obj* object = vm->grayStack[--vm->grayCount];
        blackenObject(object, vm);
    }

}
static void HashmapRemoveWhite(Hashmap* map, struct Vm* vm)
{
    for (int i = 0; i < map->capacity; i++)
    {
        Bucket* bucket = &map->buckets[i];
        if (bucket->key != NULL && !bucket->key->obj.isMarked)
        {
            MapDelete(map, bucket->key);
        }

    }
}

//actual cleanup
static void sweep(struct Vm* vm)
{
    Obj* prev = NULL;
    Obj* current = vm->objects;

    while (current != NULL)
    {
        if (current->isMarked)
        {
            current->isMarked = false;
            prev = current;
            current = current->next;
        }
        else
        {
            Obj* unreachable = current;
            current = current->next;
            if (prev != NULL)
            {
                prev->next = current;
            }
            else
            {
                vm->objects = current;
            }

            freeObject(unreachable, vm);
        }
    }
}

//main function
void collectGarbage(struct Vm* vm)
{
#ifdef DEBUG_LOG_GC  size_t before = vm.bytesAllocated;
    printf("-- gc begin\n");
    size_t before = vm->bytesAllocated;
#endif

    markRoots(vm);
    traceRefrences(vm);
    HashmapRemoveWhite(&vm->strings, vm);
    sweep(vm);

    vm->nextGC = vm->bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
    before - vm->bytesAllocated, before, vm->bytesAllocated,
    vm->nextGC);
#endif
}