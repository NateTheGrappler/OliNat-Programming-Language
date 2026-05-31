//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_EXPR_H
#define OLI_NAT_EXPR_H

#include "memory.h"

typedef enum {
    EXPR_LITERAL,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUPING,
    EXPR_VARIABLE, //TODO: add the implementation for this
} ExprType;

typedef enum {
    VALUE_BOOL,
    VALUE_INT,
    VALUE_STRING,
    VALUE_DOUBLE,
    VALUE_FLOAT,
} ValueType;

struct Expr;

typedef struct
{
    union
    {
        int integer_val;
        double double_val;
        float float_val;
        //TODO: handle strings later
        //TODO: handle booleans later as well
    } value;
    ValueType type;
} Literal;

typedef struct
{
    struct Expr* left;
    struct Expr* right;
    char operator;
} Binary;

typedef struct
{
    struct Expr* right;
    char operator;
} Unary;

typedef struct
{
    struct Expr* expr;
} Grouping;


typedef struct Expr {
    ExprType type;
    union {
        Literal literal;
        Binary binary;
        Unary unary;
        Grouping grouping;
    };
} Expr;

Expr* createLiteralDouble(double value);
Expr* createLiteralFloat (float value);
Expr* createLiteralInt   (int value);
Expr* createUnary        (char operator, Expr* right);
Expr* createBinary       (Expr* left, Expr* right, char operator);
Expr* createGrouping     (Expr* expr);

void freeExpr(Expr* expr);

#endif //OLI_NAT_EXPR_H
