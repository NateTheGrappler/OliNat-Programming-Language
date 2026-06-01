//
// Created by natang on 5/31/26.
//

#ifndef OLI_NAT_TYPECHECKER_H
#define OLI_NAT_TYPECHECKER_H

#include "Expr.h"
#include "common.h"
#include "debug.h"

//main struct to keep track of errors and eventually vars
typedef struct
{
    bool hadError;
    int errorCount;
    //TODO: add in a symbol table later on for variables
} TypeChecker;


//function to do the checking
ValueType checkExpression(TypeChecker* checker, Expr* expr);

ValueType checkLiteral(Expr* expr);
ValueType checkGrouping(TypeChecker* checker, Expr* expr);
ValueType checkUnary(TypeChecker* checker, Expr* expr);
ValueType checkBinary(TypeChecker* checker, Expr* expr);
void initTypeChecker(TypeChecker* checker);


#endif //OLI_NAT_TYPECHECKER_H
