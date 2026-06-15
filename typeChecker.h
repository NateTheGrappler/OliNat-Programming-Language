//
// Created by natang on 5/31/26.
//

#ifndef OLI_NAT_TYPECHECKER_H
#define OLI_NAT_TYPECHECKER_H
#define MAX_SYMBOLS 65536


#include "Expr.h"
#include "common.h"
#include "debug.h"
#include "object.h"

typedef struct ASTparser;

typedef struct
{
    const char* name;
    int length;
    ValueType   type;
    int depth;
    ObjFunction* function;
} Symbol;


//main struct to keep track of errors and eventually vars
typedef struct
{
    bool hadError;
    int errorCount;
    Symbol symbols[MAX_SYMBOLS];
    int varCount;
} TypeChecker;


//function to do the checking
ValueType checkExpression(TypeChecker* checker, Expr* expr, struct ASTparser* parser);

ValueType checkLiteral(Expr* expr);
ValueType checkGrouping(TypeChecker* checker, Expr* expr, struct ASTparser* parser);
ValueType checkUnary(TypeChecker* checker, Expr* expr, struct ASTparser* parser);
ValueType checkBinary(TypeChecker* checker, Expr* expr, struct ASTparser* parser);
void initTypeChecker(TypeChecker* checker);

Symbol* lookUpSymbol(TypeChecker* checker, const char* name, int length);
void addSymbol(TypeChecker* checker, const char* name, int length, int depth, ValueType type, ObjFunction* function, struct ASTparser* parser);



#endif //OLI_NAT_TYPECHECKER_H
