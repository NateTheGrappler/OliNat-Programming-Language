//
// Created by natang on 6/1/26.
//
#include "chunk.h"
#include "vm.h"

void initChunk(Chunk* chunk)
{
    chunk-> count=0;
    chunk->capacity=0;
    chunk->byteCode=NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}
void writeToChunk(Chunk* chunk, uint8_t byte, int line, struct  Vm* vm)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->byteCode =  GROW_ARRAY(uint8_t, chunk->byteCode, oldCapacity, chunk->capacity, vm);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity, vm);
    }

    chunk->byteCode[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}
void freeChunk(Chunk* chunk, struct  Vm* vm)
{
    FREE_ARRAY(uint8_t, chunk->byteCode, chunk->capacity, vm);
    FREE_ARRAY(int, chunk->lines, chunk->capacity, vm);
    freeValueArray(&chunk->constants, vm);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value, Vm* vm)
{
    push(vm, value); //for gc sake
    writeValueArray(&chunk->constants, value, vm);
    pop(vm);
    return chunk->constants.count - 1;
}