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
Expr* createLiteralBool(bool value, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.boolean_val = value;
    expr->literal.type = VALUE_BOOL;
    return expr;
}
Expr* createLiteralString(char* value, int line)
{
    printf("Creating string literal: '%s' (length %zu)\n", value, strlen(value));

    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.string_val = value;
    expr->literal.type = VALUE_STRING;
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

Expr* createVariable(const char* name, int length, int line)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->variable.length = length;
    expr->variable.name = name;
    return expr;
}
Expr* createVarAssignment(char* name, int length, Expr* value, int line)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_ASSIGN;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->var_assignment.length = length;
    expr->var_assignment.name = name;
    expr->var_assignment.value = value;
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
Expr* createCall(Expr* callee, Expr** args, int argCount,  int line)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->line = line;

    expr->objectCall.argCount = argCount;
    expr->objectCall.args = args;
    expr->objectCall.callee = callee;
    return expr;
}

Expr* createStaticArray(Expr** args, int count, ValueType type, int line)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_STATIC_ARRAY;
    expr->line = line;

    expr->staticArray.length = count;
    expr->staticArray.values = args;
    expr->staticArray.type = type;
    return expr;
}
Expr* createArraySet(Expr* left, Expr* index, Expr* value, int line)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_SET_ARRAY_INDEX;
    expr->line = line;

    expr->setArray.left = left;
    expr->setArray.index = index;
    expr->setArray.value = value;
    return expr;
}
Expr* createArrayGet(Expr* left, Expr* index, int line)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr));
    expr->type = EXPR_GET_ARRAY_INDEX;
    expr->line = line;

    expr->getArray.left = left;
    expr->getArray.index = index;
    return expr;
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
            if (expr->literal.type == VALUE_STRING && expr->literal.value.string_val != NULL)
            {
                FREE_ARRAY(char, expr->literal.value.string_val, strlen(expr->literal.value.string_val) + 1);
            }
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
        case EXPR_ASSIGN:
        {
            freeExpr(expr->var_assignment.value);
            break;
        }
        case EXPR_VARIABLE:
        {
            //name points to source string so dont free
            break;
        }
        case EXPR_CALL:
        {
            freeExpr(expr->objectCall.callee);
            for (int i = 0; i < expr->objectCall.argCount; i++)
            {
                freeExpr(expr->objectCall.args[i]);
            }
            FREE_ARRAY(Expr*, expr->objectCall.args, expr->objectCall.argCount);
            break;
        }
        case EXPR_STATIC_ARRAY:
        {
            for (int i = 0; i < expr->staticArray.length; i++)
            {
                freeExpr(expr->staticArray.values[i]);
            }
            FREE_ARRAY(Expr*, expr->staticArray.values, expr->staticArray.length);
            break;
        }
        case EXPR_GET_ARRAY_INDEX:
        {
            freeExpr(expr->getArray.index);
            freeExpr(expr->getArray.left);
            break;
        }
        case EXPR_SET_ARRAY_INDEX:
        {
            freeExpr(expr->setArray.index);
            freeExpr(expr->setArray.left);
            freeExpr(expr->setArray.value);
            break;
        }
    }

#ifdef DEBUG_TRACE_EXECUTION
    printf("freeing expr type: %d\n", expr->type);
#endif
    //the call to free the actual expression from wherever you are in the recursion
    FREE(Expr, expr);
}
