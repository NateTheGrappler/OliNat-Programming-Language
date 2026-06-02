//
// Created by natang on 6/1/26.
//
#include "chunk.h"


void initChunk(Chunk* chunk)
{
    chunk-> count=0;
    chunk->capacity=0;
    chunk->byteCode=NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}
void writeToChunk(Chunk* chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->byteCode =  GROW_ARRAY(uint8_t, chunk->byteCode, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->byteCode[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}
void freeChunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->byteCode, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value, struct Vm* vm)
{
    // push(vm, value); //for gc sake
    // writeValueArray(&chunk->constants, value);
    // pop(vm);
}