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
    VALUE_EMPTY,
    VALUE_ERROR,
} ValueTypeExpr;

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
    ValueTypeExpr type;
} Literal;

typedef struct
{
    struct Expr* left;
    struct Expr* right;
    const char* operator;
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
    int line;
    union {
        Literal literal;
        Binary binary;
        Unary unary;
        Grouping grouping;
    };
} Expr;

Expr* createLiteralDouble(double value, int line);
Expr* createLiteralFloat (float value, int line);
Expr* createLiteralInt   (int value, int line);
Expr* createUnary        (char operator, Expr* right, int line);
Expr* createBinary       (Expr* left, Expr* right, const char* operator, int line);
Expr* createGrouping     (Expr* expr, int line);

void freeExpr(Expr* expr);

#endif //OLI_NAT_EXPR_H
