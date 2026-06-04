//
// Created by natang on 6/1/26.
//

#ifndef OLI_NAT_VALUE_H
#define OLI_NAT_VALUE_H
#include "common.h"
#include "Expr.h"

typedef struct Obj Obj;

//value representation of whatever value
typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double double_val;
        float float_val;
        int   int_val;
        Obj*  object_val;
    }as;
} Value;

//actual value array held by chunk
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;


//TODO: add strings and empty to the macros
//a bunch of macros for creating objects
#define CREATE_BOOL_VAL(value)   ((Value){.type = VALUE_BOOL,   .as.boolean    = value})
#define CREATE_INT_VAL(value)    ((Value){.type = VALUE_INT,    .as.int_val    = value})
#define CREATE_FLOAT_VAL(value)  ((Value){.type = VALUE_FLOAT,  .as.float_val  = value})
#define CREATE_DOUBLE_VAL(value) ((Value){.type = VALUE_DOUBLE, .as.double_val = value})
#define CREATE_OBJECT_VAL(value) ((Value){.type = VALUE_OBJECT, .as.object_val = value})
#define CREATE_EMPTY_VAL()       ((Value){.type = VALUE_EMPTY,  .as.int_val    = 0})

//type checking macros for safety
#define IS_BOOL(value)   ((value).type == VALUE_BOOL)
#define IS_OBJECT(value) ((value).type == VALUE_OBJECT)
#define IS_INT(value)    ((value).type == VALUE_INT)
#define IS_FLOAT(value)  ((value).type == VALUE_FLOAT)
#define IS_DOUBLE(value) ((value).type == VALUE_DOUBLE)
#define IS_EMPTY(value)  ((value).type == VALUE_EMPTY)

//getting the raw c values from the struct
#define GET_BOOL_VAL(value)   ((value).as.boolean)
#define GET_INT_VAL(value)    ((value).as.int_val)
#define GET_FLOAT_VAL(value)  ((value).as.float_val)
#define GET_DOUBLE_VAL(value) ((value).as.double_val)
#define GET_OBJECT_VAL(value) ((value).as.object_val)

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);


//TODO: add null and strings
#endif //OLI_NAT_VALUE_H

