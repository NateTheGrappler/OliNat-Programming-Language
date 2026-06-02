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
ValueTypeExpr checkExpression(TypeChecker* checker, Expr* expr);

ValueTypeExpr checkLiteral(Expr* expr);
ValueTypeExpr checkGrouping(TypeChecker* checker, Expr* expr);
ValueTypeExpr checkUnary(TypeChecker* checker, Expr* expr);
ValueTypeExpr checkBinary(TypeChecker* checker, Expr* expr);
void initTypeChecker(TypeChecker* checker);


#endif //OLI_NAT_TYPECHECKER_H
