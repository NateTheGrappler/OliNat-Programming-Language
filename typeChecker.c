//
// Created by natang on 5/31/26.
//
#include "typeChecker.h"

#include "ASTcompiler.h"


void initTypeChecker(TypeChecker* checker)
{
    checker->hadError = false;
    checker->errorCount = 0;
    checker->varCount = 0;
}



//-----------------Helper Functions--------------//
static bool isNumeric(ValueType type)
{
    return type == VALUE_INT || type == VALUE_DOUBLE || type == VALUE_FLOAT;
}

static void typeError(TypeChecker* checker, Expr* expr, const char* message)
{
    fprintf(stderr, "[Error at Line %d: %s] \n", expr->line, message);
    checker->hadError = true;
    checker->errorCount++;
}

static ValueType checkBinaryReturnType(ValueType right, ValueType left)
{
    //return same types, then float piority, otherwise it has to be a double
    if (right == left) return right;
    if ((right == VALUE_FLOAT && left == VALUE_INT) || (left == VALUE_FLOAT && right == VALUE_INT)) return VALUE_FLOAT;
    return VALUE_DOUBLE;
}

//-------------------------------Functions for checking different expression types---------------------------//
ValueType checkLiteral(Expr* expr)
{
    //just return the already determined type
    return expr->literal.type;
}

ValueType checkGrouping(TypeChecker* checker, Expr* expr)
{
    //just pass it forward
    return checkExpression(checker, expr->grouping.expr);
}
ValueType checkUnary(TypeChecker* checker, Expr* expr)
{
    ValueType typeRight = checkExpression(checker, expr->unary.right);

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
ValueType checkBinary(TypeChecker* checker, Expr* expr)
{
    ValueType rightType = checkExpression(checker, expr->binary.right);
    ValueType leftType = checkExpression(checker, expr->binary.left);
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
        typeError(checker, expr, "Operators comparing value must compare numbers, booleans, or strings, you cannot mix types (sorry)."); //TODO: maybe add checking strings too idk
        return VALUE_ERROR;
    }

    //unreachable (HOPEFULLY)
    typeError(checker, expr, "Unknown binary operator, what on earth are you doing?");
    return VALUE_ERROR;

}

Symbol* lookUpSymbol(TypeChecker* checker, const char* name, int length)
{
    //traverse the array (backwards for locals) for now
    for (int i = checker->varCount - 1; i >= 0; i--)
    {
        if (checker->symbols[i].length == length &&
            memcmp(checker->symbols[i].name, name, length) == 0)
            return &checker->symbols[i];
    }
    return NULL;
}
void addSymbol(TypeChecker* checker, const char* name, int length, int depth, ValueType type, ASTparser* parser)
{
    if (checker->varCount >= 256)
    {
        //TODO: have it so you can store more vars but also have a seperate error system for types
        error("Too many variables have been declared.", parser);
        return;
    }
    int index = checker->varCount++;
    checker->symbols[index].type = type;
    checker->symbols[index].name = name;
    checker->symbols[index].length = length;
    checker->symbols[index].depth = depth;
}
ValueType checkVariable(TypeChecker* checker, Expr* expr)
{
    Symbol* symbol = lookUpSymbol(checker, expr->variable.name, expr->variable.length);
    if (symbol != NULL)
    {
        return symbol->type;
    }
    typeError(checker, expr, "Undefined variable.");
    return VALUE_ERROR;
}
//-------------------------------Main function for entry-----------------------------------------//

ValueType checkExpression(TypeChecker* checker, Expr* expr)
{
#ifdef DEBUG_TRACE_EXECUTION
    printf("Checking new expression: | ");
    printExpression(expr);
    printf(" | expression type: %d", expr->type);
    printf("\n");
#endif

    if (expr == NULL) return VALUE_ERROR;

    
    ValueType result;
    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            result = checkBinary(checker, expr);
            break;
        }
        case EXPR_LITERAL:
        {
            result = checkLiteral(expr);
            break;
        }
        case EXPR_UNARY:
        {
            if (expr->unary.right == NULL)
            {
                return VALUE_ERROR;
            }
            result = checkUnary(checker, expr);
            break;
        }
        case EXPR_GROUPING:
        {
            result = checkGrouping(checker, expr);
            break;
        }
        case EXPR_VARIABLE:
        {
            result = checkVariable(checker, expr);
            break;
        }
        case EXPR_ASSIGN:
        {
            Symbol* symbol = lookUpSymbol(checker, expr->var_assignment.name, expr->var_assignment.length);
            if (symbol == NULL)
            {
                typeError(checker,  expr, "You cannot try to reassign a var that does not exist, no having your cake and eating it.");
                return VALUE_ERROR;
            }
            ValueType type = checkExpression(checker, expr->var_assignment.value);
            if (type != symbol->type)
            {
                typeError(checker, expr, "You cannot try to give a different type to an existing variable!");
                return VALUE_ERROR;
            }
            result = symbol->type;
            break;
        }
        default:
            result =  VALUE_ERROR;
            break;
    }
    // #ifdef DEBUG_TRACE_EXECUTION
    //     printf("----------------------EXPRESSION ENDED----------------------");
    //     printf("\n");
    // #endif

    return result;
}