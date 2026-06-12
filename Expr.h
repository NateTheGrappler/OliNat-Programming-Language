//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_EXPR_H
#define OLI_NAT_EXPR_H

#include "memory.h"
#include "common.h"

typedef enum {
    EXPR_LITERAL,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUPING,
    EXPR_ASSIGN,
    EXPR_VARIABLE,
    EXPR_CALL,
    EXPR_STATIC_ARRAY
} ExprType;

struct Expr;
typedef struct
{
    struct Expr* callee; //var holding funciton or class
    struct Expr** args;
    int argCount;
} Call;
typedef struct
{
    const char* name;
    int length;
} Variable;
typedef struct
{
    struct Expr** values;
    ValueType type;
    int length;
} staticArray;
typedef struct
{
    const char* name;
    int length;
    struct Expr* value;
} VarAssignment;
typedef struct
{
    union
    {
        int integer_val;
        double double_val;
        float float_val;
        bool boolean_val;
        char* string_val;
    } value;
    ValueType type;
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
        Variable variable;
        Grouping grouping;
        VarAssignment var_assignment;
        Call objectCall;
        staticArray staticArray;
    };
} Expr;

Expr* createLiteralDouble(double value, int line);
Expr* createLiteralFloat (float value, int line);
Expr* createLiteralInt   (int value, int line);
Expr* createLiteralBool  (bool value, int line);
Expr* createLiteralString(char* value, int line);
Expr* createVarAssignment(char* name, int length, Expr* value, int line);
Expr* createStaticArray(Expr** args, int count, ValueType type, int line);
Expr* createUnary        (char operator, Expr* right, int line);
Expr* createBinary       (Expr* left, Expr* right, const char* operator, int line);
Expr* createCall         (Expr* callee, Expr** args, int argCount,  int line);
Expr* createGrouping     (Expr* expr, int line);
Expr* createVariable(const char* name, int length, int line);

void freeExpr(Expr* expr);

#endif //OLI_NAT_EXPR_H
