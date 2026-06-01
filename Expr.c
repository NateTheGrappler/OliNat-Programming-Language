//
// Created by natang on 5/30/26.
//
#include "Expr.h"
#include "common.h"

//------------------Basic initializers for expressions------------------------//

Expr* createLiteralDouble(double value, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.double_val = value;
    expr->literal.type = VALUE_DOUBLE;
    return expr;
}
Expr* createLiteralFloat(float value, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.float_val = value;
    expr->literal.type = VALUE_FLOAT;
    return expr;
}
Expr* createLiteralInt(int value, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.integer_val = value;
    expr->literal.type = VALUE_INT;
    return expr;
}
Expr* createUnary(char operator, Expr* right, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0,  sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->line = line;

    expr->unary.operator = operator;
    expr->unary.right = right;
    return expr;
}
Expr* createBinary(Expr* left, Expr* right, const char* operator, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->line = line;

    expr->binary.operator = operator;
    expr->binary.right = right;
    expr->binary.left = left;
    return expr;
}
Expr* createGrouping(Expr* expr, int line)
{
    Expr* exprG = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    exprG->type = EXPR_GROUPING;
    exprG->line = line;

    exprG->grouping.expr = expr;
    return exprG;
}

void freeExpr(Expr* expr)
{
    //switch statement that frees the values based on their type
    ////recursively frees the data for any child nodes as well
    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            freeExpr(expr->binary.left);
            freeExpr(expr->binary.right);
            break;
        }
        case EXPR_LITERAL:
        {
            //TODO: when strings exist, free them in here
            break;
        }
        case EXPR_UNARY:
        {
            freeExpr(expr->unary.right);
            break;
        }
        case EXPR_GROUPING:
        {
            freeExpr(expr->grouping.expr);
            break;
        }
    }

    //the call to free the actual expression from wherever you are in the recursion
    FREE(Expr, expr);
}
