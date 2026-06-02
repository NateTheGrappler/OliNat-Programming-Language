//
// Created by natang on 5/31/26.
//
#include "typeChecker.h"


void initTypeChecker(TypeChecker* checker)
{
    checker->hadError = false;
    checker->errorCount = 0;
    //TODO: add in symbol table alter
}



//-----------------Helper Functions--------------//
static bool isNumeric(ValueTypeExpr type)
{
    return type == VALUE_INT || type == VALUE_DOUBLE || type == VALUE_FLOAT;
}

static void typeError(TypeChecker* checker, Expr* expr, const char* message)
{
    fprintf(stderr, "[Error at Line %d: %s] \n", expr->line, message);
    checker->hadError = true;
    checker->errorCount++;
}

static ValueTypeExpr checkBinaryReturnType(ValueTypeExpr right, ValueTypeExpr left)
{
    //return same types, then float piority, otherwise it has to be a double
    if (right == left) return right;
    if ((right == VALUE_FLOAT && left == VALUE_INT) || (left == VALUE_FLOAT && right == VALUE_INT)) return VALUE_FLOAT;
    return VALUE_DOUBLE;
}

//-------------------------------Functions for checking different expression types---------------------------//
ValueTypeExpr checkLiteral(Expr* expr)
{
    //just return the already determined type
    return expr->literal.type;
}
ValueTypeExpr checkGrouping(TypeChecker* checker, Expr* expr)
{
    //just pass it forward
    return checkExpression(checker, expr->grouping.expr);
}
ValueTypeExpr checkUnary(TypeChecker* checker, Expr* expr)
{
    ValueTypeExpr typeRight = checkExpression(checker, expr->binary.right);

    //handle unary cases based on their operator
    switch (expr->unary.operator)
    {
        case '-':
        {
            //check for right operand and then return an error if it's wrong
            if (isNumeric(typeRight)) { return typeRight; }
            typeError(checker, expr, "When trying to negate a value, please use a number."); //toss error if not number
            return VALUE_ERROR;
        }
        case '!':
        {
            if (typeRight == VALUE_BOOL) { return typeRight; }
            typeError(checker, expr, "Operator '!' only works on boolean values man!");
            return VALUE_ERROR;
        }
        default:
        {
            typeError(checker, expr, "Unknown unary expression syntax, what are you doing?");
            return VALUE_ERROR;
        }
    }
}
ValueTypeExpr checkBinary(TypeChecker* checker, Expr* expr)
{
    ValueTypeExpr rightType = checkExpression(checker, expr->binary.right);
    ValueTypeExpr leftType = checkExpression(checker, expr->binary.left);
    const char* operator = expr->binary.operator; //it's a 2 char array so it's 2 bytes

    //do the arithemtiatic expressions first
    if (strcmp(operator, "+") == 0 ||
        strcmp(operator, "-") == 0 ||
        strcmp(operator, "/") == 0 ||
        strcmp(operator, "*") == 0 )
    {
        if (isNumeric(leftType) && isNumeric(rightType)) return checkBinaryReturnType(rightType, leftType); //somehow handle different expression outputs
        typeError(checker, expr, "If you want to do math, you have to use numbers (double, float, int), please.");
        return VALUE_ERROR;
    }

    //handle arithemtitec boolean operators
    if (strcmp(operator, "<") == 0 ||
        strcmp(operator, ">") == 0 ||
        strcmp(operator, "<=") == 0 ||
        strcmp(operator, ">=") == 0 )
    {
        if (isNumeric(leftType) && isNumeric(rightType)) return VALUE_BOOL; //always a boolean
        typeError(checker, expr, "Operators comparing value must compare numbers only."); //TODO: maybe add checking strings too idk
        return VALUE_ERROR;
    }

    //handle equality operator
    if (strcmp(operator, "==") == 0 || strcmp(operator, "!=") == 0)
    {
        if (leftType == rightType) return VALUE_BOOL;
        if (isNumeric(leftType) && isNumeric(rightType)) return VALUE_BOOL;
        typeError(checker, expr, "Operators comparing value must compare numbers only."); //TODO: maybe add checking strings too idk
        return VALUE_ERROR;
    }

    //unreachable (HOPEFULLY)
    typeError(checker, expr, "Unknown binary operator, what on earth are you doing?");
    return VALUE_ERROR;

}




//-------------------------------Main function for entry-----------------------------------------//

ValueTypeExpr checkExpression(TypeChecker* checker, Expr* expr)
{
    //TODO: delete debug code
    printf(" \n Checking new expression: ");
    printExpression(expr);
    //printf(" \n ");


    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            return checkBinary(checker, expr);
        }
        case EXPR_LITERAL:
        {
            return checkLiteral(expr);
        }
        case EXPR_UNARY:
        {
            return checkUnary(checker, expr);
        }
        case EXPR_GROUPING:
        {
            return checkGrouping(checker, expr);
        }
            default:
            return VALUE_ERROR;
    }
}