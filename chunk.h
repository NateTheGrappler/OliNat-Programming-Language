//
// Created by natang on 6/1/26.
//

#ifndef OLI_NAT_CHUNK_H
#define OLI_NAT_CHUNK_H
#include "common.h"
#include "Value.h"
#include "memory.h"

struct Vm; //forward declarationso the c compiler doesnt blow up

typedef enum
{
    OP_ADD,
    OP_MULTIPLY,
    OP_SUBTRACT,
    OP_NEGATE,
    OP_DIVIDE,
    OP_RETURN,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS
} OpCode;

//TODO: add in more stack space, or at least bytecode value space
typedef struct
{
    int count;
    int capacity;
    int* lines;
    uint8_t* byteCode;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeToChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value, struct Vm* vm); //add in a value to the value array and return the index of that value

#endif //OLI_NAT_CHUNK_H
