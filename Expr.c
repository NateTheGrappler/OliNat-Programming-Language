//
// Created by natang on 5/30/26.
//
#include "Expr.h"
#include "common.h"

//------------------Basic initializers for expressions------------------------//

Expr* createLiteralDouble(double value, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.double_val = value;
    expr->literal.type = VALUE_DOUBLE;
    return expr;
}
Expr* createLiteralBool(bool value, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.boolean_val = value;
    expr->literal.type = VALUE_BOOL;
    return expr;
}
Expr* createLiteralString(char* value, int line, struct Vm* vm)
{
#ifdef DEBUG_TRACE_EXECUTION
    printf("Creating string literal: '%s' (length %zu)\n", value, strlen(value));
#endif

    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_LITERAL;
    expr->line = line;
    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.string_val = value;
    expr->literal.type = VALUE_STRING;
    return expr;
}
Expr* createLiteralFloat(float value, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.float_val = value;
    expr->literal.type = VALUE_FLOAT;
    return expr;
}
Expr* createLiteralInt(int value, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_LITERAL;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->literal.value.integer_val = value;
    expr->literal.type = VALUE_INT;
    return expr;
}

Expr* createVariable(const char* name, int length, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_VARIABLE;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->variable.length = length;
    expr->variable.name = name;
    return expr;
}
Expr* createVarAssignment(char* name, int length, Expr* value, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_ASSIGN;
    expr->line = line;

    //call the anyonmous union that holds all the different possible expressions
    expr->var_assignment.length = length;
    expr->var_assignment.name = name;
    expr->var_assignment.value = value;
    return expr;
}

Expr* createUnary(char operator, Expr* right, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0,  sizeof(Expr), vm);
    expr->type = EXPR_UNARY;
    expr->line = line;

    expr->unary.operator = operator;
    expr->unary.right = right;
    return expr;
}
Expr* createBinary(Expr* left, Expr* right, const char* operator, int line, struct Vm* vm)
{
    //allocate the expr in memory and set it's type
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_BINARY;
    expr->line = line;

    expr->binary.operator = operator;
    expr->binary.right = right;
    expr->binary.left = left;
    return expr;
}
Expr* createOr(Expr* left, Expr* right, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_OR;
    expr->line = line;

    expr->binary.right = right;
    expr->binary.left = left;
    return expr;
}
Expr* createAnd(Expr* left, Expr* right, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_AND;
    expr->line = line;

    expr->binary.right = right;
    expr->binary.left = left;
    return expr;
}
Expr* createGrouping(Expr* expr, int line, struct Vm* vm)
{
    Expr* exprG = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    exprG->type = EXPR_GROUPING;
    exprG->line = line;

    exprG->grouping.expr = expr;
    return exprG;
}
Expr* createCall(Expr* callee, Expr** args, int argCount,  int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_CALL;
    expr->line = line;

    expr->objectCall.argCount = argCount;
    expr->objectCall.args = args;
    expr->objectCall.callee = callee;
    return expr;
}

Expr* createStaticArray(Expr** args, int count, ValueType type, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_STATIC_ARRAY;
    expr->line = line;

    expr->staticArray.length = count;
    expr->staticArray.values = args;
    expr->staticArray.type = type;
    return expr;
}
Expr* createArraySet(Expr* left, Expr* index, Expr* value, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_SET_ARRAY_INDEX;
    expr->line = line;

    expr->setArray.left = left;
    expr->setArray.index = index;
    expr->setArray.value = value;
    return expr;
}
Expr* createArrayGet(Expr* left, Expr* index, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_GET_ARRAY_INDEX;
    expr->line = line;

    expr->getArray.left = left;
    expr->getArray.index = index;
    return expr;
}

Expr* createGetField(Expr* callee, const char* fieldName, int fieldLength, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_GET_FIELD;
    expr->line = line;

    expr->getField.callee = callee;
    expr->getField.fieldLenght = fieldLength;
    expr->getField.fieldName = fieldName;
    return expr;
}
Expr* createSetField(Expr* callee, Expr* newValue, const char* fieldName, int fieldLength, int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_SET_FIELD;
    expr->line = line;

    expr->setField.callee = callee;
    expr->setField.newValue = newValue;
    expr->setField.fieldLenght = fieldLength;
    expr->setField.fieldName = fieldName;
    return expr;
}
Expr* createThisExpr(int line, struct Vm* vm)
{
    Expr* expr = (Expr*)reallocate(NULL, 0, sizeof(Expr), vm);
    expr->type = EXPR_THIS;
    expr->line = line;
    return expr;
}

void freeExpr(Expr* expr, struct Vm* vm)
{
    //switch statement that frees the values based on their type
    ////recursively frees the data for any child nodes as well
    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            freeExpr(expr->binary.left, vm);
            freeExpr(expr->binary.right, vm);
            break;
        }
        case EXPR_AND:
        {
            freeExpr(expr->andExpr.left, vm);
            freeExpr(expr->andExpr.right, vm);
            break;
        }
        case EXPR_OR:
        {
            freeExpr(expr->orExpr.left, vm);
            freeExpr(expr->orExpr.right, vm);
            break;
        }
        case EXPR_LITERAL:
        {
            if (expr->literal.type == VALUE_STRING && expr->literal.value.string_val != NULL)
            {
                FREE_ARRAY(char, expr->literal.value.string_val, strlen(expr->literal.value.string_val) + 1, vm);
            }
            break;
        }
        case EXPR_UNARY:
        {
            freeExpr(expr->unary.right, vm);
            break;
        }
        case EXPR_GROUPING:
        {
            freeExpr(expr->grouping.expr, vm);
            break;
        }
        case EXPR_ASSIGN:
        {
            freeExpr(expr->var_assignment.value, vm);
            break;
        }
        case EXPR_VARIABLE:
        {
            //name points to source string so dont free
            break;
        }
        case EXPR_CALL:
        {
            freeExpr(expr->objectCall.callee, vm);
            for (int i = 0; i < expr->objectCall.argCount; i++)
            {
                freeExpr(expr->objectCall.args[i], vm);
            }
            FREE_ARRAY(Expr*, expr->objectCall.args, expr->objectCall.argCount, vm);
            break;
        }
        case EXPR_STATIC_ARRAY:
        {
            for (int i = 0; i < expr->staticArray.length; i++)
            {
                freeExpr(expr->staticArray.values[i], vm);
            }
            FREE_ARRAY(Expr*, expr->staticArray.values, expr->staticArray.length, vm);
            break;
        }
        case EXPR_GET_ARRAY_INDEX:
        {
            freeExpr(expr->getArray.index, vm);
            freeExpr(expr->getArray.left, vm);
            break;
        }
        case EXPR_SET_ARRAY_INDEX:
        {
            freeExpr(expr->setArray.index, vm);
            freeExpr(expr->setArray.left, vm);
            freeExpr(expr->setArray.value, vm);
            break;
        }
        case EXPR_SET_FIELD:
        {
            freeExpr(expr->setField.newValue, vm);
            freeExpr(expr->setField.callee, vm);
            break;
        }
        case EXPR_GET_FIELD:
        {
            freeExpr(expr->getField.callee, vm);
            break;
        }
        case EXPR_THIS:
        {
            break; //no real data to free
        }
    }

#ifdef DEBUG_TRACE_EXECUTION
    printf("freeing expr type: %d\n", expr->type);
#endif
    //the call to free the actual expression from wherever you are in the recursion
    FREE(Expr, expr, vm);
}
