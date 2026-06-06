//
// Created by natang on 5/31/26.
//

#ifndef OLI_NAT_TYPECHECKER_H
#define OLI_NAT_TYPECHECKER_H

#include "Expr.h"
#include "common.h"
#include "debug.h"

typedef struct ASTparser;

typedef struct
{
    const char* name;
    int length;
    ValueType   type;
    int depth;
} Symbol;


//main struct to keep track of errors and eventually vars
typedef struct
{
    bool hadError;
    int errorCount;
    Symbol symbols[256];
    int varCount;
} TypeChecker;


//function to do the checking
ValueType checkExpression(TypeChecker* checker, Expr* expr);

ValueType checkLiteral(Expr* expr);
ValueType checkGrouping(TypeChecker* checker, Expr* expr);
ValueType checkUnary(TypeChecker* checker, Expr* expr);
ValueType checkBinary(TypeChecker* checker, Expr* expr);
void initTypeChecker(TypeChecker* checker);

Symbol* lookUpSymbol(TypeChecker* checker, const char* name, int length);
void addSymbol(TypeChecker* checker, const char* name, int length, int depth, ValueType type, struct ASTparser* parser);



#endif //OLI_NAT_TYPECHECKER_H
