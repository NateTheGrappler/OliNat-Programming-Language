//
// Created by natang on 6/1/26.
//

#ifndef OLI_NAT_VALUE_H
#define OLI_NAT_VALUE_H
#include "common.h"

//types of values for safer runtime checking
typedef enum
{
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_DOUBLE,
    VAL_EMPTY,
    VAL_OBJ, //eventually for strings and other objects
} ValueTypeVM;


//value representation of whatever value
typedef struct
{
    ValueTypeVM type;
    union
    {
        bool boolean;
        double double_val;
        float float_val;
        int   int_val;
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
#define CREATE_BOOL_VAL(value)   ((Value){.type = VAL_BOOL,   .as.boolean    = value})
#define CREATE_INT_VAL(value)    ((Value){.type = VAL_INT,    .as.int_val    = value})
#define CREATE_FLOAT_VAL(value)  ((Value){.type = VAL_FLOAT,  .as.float_val  = value})
#define CREATE_DOUBLE_VAL(value) ((Value){.type = VAL_DOUBLE, .as.double_val = value})
#define CREATE_EMPTY_VAL()       ((Value){.type = VAL_EMPTY,  .as.int_val    = 0})

//type checking macros for safety
#define IS_BOOL(value)   ((value).type == VAL_BOOL)
#define IS_INT(value)    ((value).type == VAL_INT)
#define IS_FLOAT(value)  ((value).type == VAL_FLOAT)
#define IS_DOUBLE(value) ((value).type == VAL_DOUBLE)
#define IS_EMPTY(value)  ((value).type == VAL_EMPTY)

//getting the raw c values from the struct
#define GET_BOOL_VAL(value)   ((value).as.boolean)
#define GET_INT_VAL(value)    ((value).as.int_val)
#define GET_FLOAT_VAL(value)  ((value).as.float_val)
#define GET_DOUBLE_VAL(value) ((value).as.double_val)

//TODO: add null and strings
#endif //OLI_NAT_VALUE_H

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
