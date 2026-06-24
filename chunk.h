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
    OP_NOT_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_POP,
    OP_INVERSE,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_CALL,
    OP_MISSING_RETURN,
    OP_CREATE_ARRAY,
    OP_GET_ARRAY_INDEX,
    OP_SET_ARRAY_INDEX,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLASS
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
void writeToChunk(Chunk* chunk, uint8_t byte, int line, struct Vm* vm);
void freeChunk(Chunk* chunk, struct  Vm* vm);
int addConstant(Chunk* chunk, Value value, struct Vm* vm); //add in a value to the value array and return the index of that value

#endif //OLI_NAT_CHUNK_H
