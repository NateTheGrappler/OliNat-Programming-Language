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
    VALUE_BOOL,
    VALUE_INT,
    VALUE_STRING,
    VALUE_OBJECT,
    VALUE_DOUBLE,
    VALUE_FLOAT,
    VALUE_EMPTY,
    VALUE_ERROR,
} ValueType;

#define DEBUG_TRACE_EXECUTION

#endif //OLI_NAT_COMMON_H
