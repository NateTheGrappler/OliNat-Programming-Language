//
// Created by natang on 5/29/26.
//

#ifndef OLI_NAT_COMMON_H
#define OLI_NAT_COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef enum {
    //basic
    VALUE_BOOL,
    VALUE_INT,
    VALUE_STRING,
    VALUE_OBJECT,
    VALUE_DOUBLE,
    VALUE_FLOAT,
    VALUE_CLASS,
    VALUE_INSTANCE,
    VALUE_EMPTY,
    //arrays
    VALUE_BOOL_ARRAY,
    VALUE_INT_ARRAY,
    VALUE_STRING_ARRAY,
    VALUE_OBJECT_ARRAY,
    VALUE_DOUBLE_ARRAY,
    VALUE_FLOAT_ARRAY,
    VALUE_EMPTY_ARRAY,
    //error
    VALUE_ERROR,
    //For maybe auto keyword in future, but only for native fns rn
    VALUE_ANY,
    VALUE_ANY_NUM,
    VALUE_ANY_ARRAY
} ValueType;

ValueType toArrayType(ValueType base);
ValueType toElementType(ValueType arrayType);


#define DEBUG_TRACE_EXECUTION
//#define DEBUG_LOG_GC
//#define DEBUG_STRESS_GC
#endif //OLI_NAT_COMMON_H
