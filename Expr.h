//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_EXPR_H
#define OLI_NAT_EXPR_H

#include "memory.h"
#include "common.h"

typedef struct Vm vm;

typedef enum {
    EXPR_LITERAL,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUPING,
    EXPR_ASSIGN,
    EXPR_VARIABLE,
    EXPR_CALL,
    EXPR_STATIC_ARRAY,
    EXPR_GET_ARRAY_INDEX,
    EXPR_SET_ARRAY_INDEX,
    EXPR_OR,
    EXPR_AND,
    EXPR_SET_FIELD,
    EXPR_GET_FIELD,
    EXPR_THIS
} ExprType;

struct Expr;
typedef struct
{
    //hold nothing, just an expr to showcase a this operation
} Exprthis;
typedef struct
{
    struct Expr* callee;
    const char* fieldName;
    int fieldLenght;
    struct Expr* newValue;
} SetField;
typedef struct
{
    struct Expr* callee;
    const char* fieldName;
    int fieldLenght;
} GetField;
typedef struct
{
    struct Expr* left;
    struct Expr* right;
} AndExpr;
typedef struct
{
    struct Expr* left;
    struct Expr* right;
} OrExpr;
typedef struct
{
    struct Expr* left;
    struct Expr* index;
} GetArray;
typedef struct
{
    struct Expr* left;
    struct Expr* index;
    struct Expr* value;
} SetArray;
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
        GetArray getArray;
        SetArray setArray;
        AndExpr andExpr;
        OrExpr orExpr;
        SetField setField;
        GetField getField;
        Exprthis _this;
    };
} Expr;

Expr* createLiteralDouble(double value, int line, struct Vm* vm);
Expr* createLiteralFloat (float value, int line, struct Vm* vm);
Expr* createLiteralInt   (int value, int line, struct Vm* vm);
Expr* createLiteralBool  (bool value, int line, struct Vm* vm);
Expr* createLiteralString(char* value, int line, struct Vm* vm);
Expr* createVarAssignment(char* name, int length, Expr* value, int line, struct Vm* vm);
Expr* createStaticArray  (Expr** args, int count, ValueType type, int line, struct Vm* vm);
Expr* createArraySet     (Expr* left, Expr* index, Expr* value, int line, struct Vm* vm);
Expr* createArrayGet     (Expr* left, Expr* index, int line, struct Vm* vm);
Expr* createUnary        (char operator, Expr* right, int line, struct Vm* vm);
Expr* createBinary       (Expr* left, Expr* right, const char* operator, int line, struct Vm* vm);
Expr* createOr           (Expr* left, Expr* right, int line, struct Vm* vm);
Expr* createAnd          (Expr* left, Expr* right, int line, struct Vm* vm);
Expr* createCall         (Expr* callee, Expr** args, int argCount,  int line, struct Vm* vm);
Expr* createGrouping     (Expr* expr, int line, struct Vm* vm);
Expr* createVariable     (const char* name, int length, int line, struct Vm* vm);
Expr* createGetField     (Expr* callee, const char* fieldName, int fieldLength, int line, struct Vm* vm);
Expr* createSetField     (Expr* callee, Expr* newValue, const char* fieldName, int fieldLength, int line, struct Vm* vm);
Expr* createThisExpr     (int line, struct Vm* vm);

void freeExpr(Expr* expr, struct Vm* vm);

#endif //OLI_NAT_EXPR_H
