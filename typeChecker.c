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

static void typeError(TypeChecker* checker, ASTparser* parser, Expr* expr, const char* message, const char* messageType)
{
    if (parser->hadError) return;

    fprintf(stderr, ":>>  | %s | at line %d: %s] \n", messageType, expr->line, message);
    checker->hadError = true;
    checker->errorCount++;

    parser->hadError = true;
}

static ValueType checkBinaryReturnType(ValueType right, ValueType left)
{
    //return same types, then float piority, otherwise it has to be a double
    if (right == left) return right;
    if ((right == VALUE_FLOAT && left == VALUE_INT) || (left == VALUE_FLOAT && right == VALUE_INT)) return VALUE_FLOAT;
    return VALUE_DOUBLE;
}

//------------------------------native / stdlib stuff -----------------------------//
static void registerNativeSymbol(TypeChecker* checker, const char* name, int length, ValueType returnType, ParamInfo* params, int paramCount, ASTparser* parser)
{
    // create a dummy ObjFunction just to hold the signature
    ObjFunction* sig = ALLOCATE(ObjFunction, 1);
    sig->returnType = returnType;
    sig->arity = paramCount;
    for (int i = 0; i < paramCount; i++)
    {
        sig->params[i] = params[i];
    }
    addSymbol(checker, name, length, 0, returnType, sig, parser);
}
void registerIOSymbols(TypeChecker* checker, struct ASTparser* parser)
{
    //print(ANY msg) -> void
    ParamInfo printParams[1];
    printParams[0].type = VALUE_ANY;
    printParams[0].name = "msg";
    printParams[0].length = 3;
    registerNativeSymbol(checker, "print", 5, VALUE_EMPTY, printParams, 1, parser);

    //println(ANY msg) -> void
    ParamInfo printlnParams[1];
    printlnParams[0].type = VALUE_ANY;
    printlnParams[0].name = "msg";
    printlnParams[0].length = 3;
    registerNativeSymbol(checker, "println", 7, VALUE_EMPTY, printlnParams, 1, parser);

    //intake(String/EMPTY prompt) -> string (the think user inputed)
    ParamInfo intakeParams[1];
    intakeParams[0].type = VALUE_ANY;
    intakeParams[0].name = "prompt";
    intakeParams[0].length = 6;
    registerNativeSymbol(checker, "intake", 6, VALUE_STRING, intakeParams, 1, parser);
}
void registerMathSymbols(TypeChecker* checker, struct ASTparser* parser)
{

    ParamInfo mathParams[1];
    mathParams[0].type = VALUE_ANY_NUM;
    mathParams[0].name = "num";
    mathParams[0].length = 3;

    //sin(double/float/int radians) -> double
    registerNativeSymbol(checker, "sin", 3, VALUE_DOUBLE, mathParams, 1, parser);
    registerNativeSymbol(checker, "cos", 3, VALUE_DOUBLE, mathParams, 1, parser);
    registerNativeSymbol(checker, "tan", 3, VALUE_DOUBLE, mathParams, 1, parser);
    registerNativeSymbol(checker, "abs", 3, VALUE_DOUBLE, mathParams, 1, parser);
}
void registerTimeSymbols(TypeChecker* checker, struct ASTparser* parser)
{
    registerNativeSymbol(checker, "clock", 5, VALUE_DOUBLE, NULL, 0, parser);
}
void registerFileIOSymbols(TypeChecker* checker, struct ASTparser* parser)
{

}
void registerTypeSymbols(TypeChecker* checker, struct ASTparser* parser)
{

}
void registerHashMapSymbols(TypeChecker* checker, struct ASTparser* parser)
{

}
void registerArrayListSymbols(TypeChecker* checker, struct ASTparser* parser)
{

}
void registerUtilsSymbols(TypeChecker* checker, struct ASTparser* parser)
{

}


//-------------------------------Functions for checking different expression types---------------------------//
ValueType checkLiteral(Expr* expr)
{
    //just return the already determined type
    return expr->literal.type;
}

ValueType checkGrouping(TypeChecker* checker, Expr* expr, ASTparser* parser)
{
    //just pass it forward
    return checkExpression(checker, expr->grouping.expr, parser);
}
ValueType checkUnary(TypeChecker* checker, Expr* expr, ASTparser* parser)
{
    ValueType typeRight = checkExpression(checker, expr->unary.right, parser);

    //handle unary cases based on their operator
    switch (expr->unary.operator)
    {
        case '-':
        {
            //check for right operand and then return an error if it's wrong
            if (isNumeric(typeRight)) { return typeRight; }
            typeError(checker, parser, expr, "When trying to negate a value, please use a number.", "TYPE ERROR"); //toss error if not number

            return VALUE_ERROR;
        }
        case '!':
        {
            if (typeRight == VALUE_BOOL) { return typeRight; }
            typeError(checker, parser, expr, "Operator '!' only works on boolean values man!" , "TYPE ERROR");
            return VALUE_ERROR;
        }
        default:
        {
            typeError(checker, parser, expr, "Unknown unary expression syntax, what are you doing?", "TYPE ERROR");
            return VALUE_ERROR;
        }
    }
}
ValueType checkBinary(TypeChecker* checker, Expr* expr, ASTparser* parser)
{
    ValueType rightType = checkExpression(checker, expr->binary.right, parser);
    ValueType leftType = checkExpression(checker, expr->binary.left, parser);
    const char* operator = expr->binary.operator; //it's a 2 char array so it's 2 bytes

    //do the arithemtiatic expressions first
    if (strcmp(operator, "+") == 0 ||
        strcmp(operator, "-") == 0 ||
        strcmp(operator, "/") == 0 ||
        strcmp(operator, "*") == 0 )
    {
        if (isNumeric(leftType) && isNumeric(rightType)) return checkBinaryReturnType(rightType, leftType); //somehow handle different expression outputs
        if (strcmp(operator, "+") == 0 && rightType == VALUE_STRING && leftType == VALUE_STRING) { return VALUE_STRING; }

        typeError(checker, parser, expr, "If you want to do math, you have to use numbers (double, float, int), please." , "TYPE ERROR");
        return VALUE_ERROR;
    }

    //handle arithemtitec boolean operators
    if (strcmp(operator, "<") == 0 ||
        strcmp(operator, ">") == 0 ||
        strcmp(operator, "<=") == 0 ||
        strcmp(operator, ">=") == 0 )
    {
        if (isNumeric(leftType) && isNumeric(rightType)) return VALUE_BOOL; //always a boolean
        typeError(checker, parser, expr, "Operators comparing value must compare numbers only.", "TYPE ERROR"); //TODO: maybe add checking strings too idk
        return VALUE_ERROR;
    }

    //handle equality operator
    if (strcmp(operator, "==") == 0 || strcmp(operator, "!=") == 0)
    {
        if (leftType == rightType) return VALUE_BOOL;
        if (isNumeric(leftType) && isNumeric(rightType)) return VALUE_BOOL;
        typeError(checker, parser, expr, "Operators comparing value must compare numbers, booleans, or strings, you cannot mix types (sorry)." , "TYPE ERROR"); //TODO: maybe add checking strings too idk
        return VALUE_ERROR;
    }

    //unreachable (HOPEFULLY)
    typeError(checker, parser, expr, "Unknown binary operator, what on earth are you doing?", "TYPE ERROR");
    return VALUE_ERROR;

}

Symbol* lookUpSymbol(TypeChecker* checker, const char* name, int length)
{
    //traverse the array (backwards for locals) for now
    #ifdef DEBUG_TRACE_EXECUTION
        printf("Looking up: %.*s, table has %d symbols\n", length, name, checker->varCount);
    #endif

    for (int i = checker->varCount - 1; i >= 0; i--)
    {

        #ifdef DEBUG_TRACE_EXECUTION
                printf("  symbol[%d]: %.*s\n", i, checker->symbols[i].length, checker->symbols[i].name);
        #endif

        if (checker->symbols[i].length == length &&
            memcmp(checker->symbols[i].name, name, length) == 0)
            return &checker->symbols[i];
    }
    return NULL;
}
void addSymbol(TypeChecker* checker, const char* name, int length, int depth, ValueType type, ObjFunction* function, ASTparser* parser)
{
    if (checker->varCount >= MAX_SYMBOLS)
    {
        error("Too many variables have been declared.", "MEMORY ERROR", parser);
        return;
    }
    int index = checker->varCount++;
    checker->symbols[index].type = type;
    checker->symbols[index].name = name;
    checker->symbols[index].length = length;
    checker->symbols[index].depth = depth;
    checker->symbols[index].function = function;
}
ValueType checkVariable(TypeChecker* checker, ASTparser* parser, Expr* expr)
{
    Symbol* symbol = lookUpSymbol(checker, expr->variable.name, expr->variable.length);
    if (symbol != NULL)
    {
        return symbol->type;
    }
    typeError(checker, parser, expr, "Undefined variable.", "UNDEFINED ERROR");
    return VALUE_ERROR;
}
//-------------------------------Main function for entry-----------------------------------------//

ValueType checkExpression(TypeChecker* checker, Expr* expr, ASTparser* parser)
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
            result = checkBinary(checker, expr, parser);
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
            result = checkUnary(checker, expr, parser);
            break;
        }
        case EXPR_GROUPING:
        {
            result = checkGrouping(checker, expr, parser);
            break;
        }
        case EXPR_VARIABLE:
        {
            result = checkVariable(checker, parser, expr);
            break;
        }
        case EXPR_ASSIGN:
        {
            Symbol* symbol = lookUpSymbol(checker, expr->var_assignment.name, expr->var_assignment.length);
            if (symbol == NULL)
            {
                typeError(checker, parser,  expr, "You cannot try to reassign a var that does not exist, no having your cake and eating it.", "UNDEFINED ERROR");
                return VALUE_ERROR;
            }
            ValueType type = checkExpression(checker, expr->var_assignment.value, parser);
            if (type != symbol->type)
            {
                typeError(checker, parser, expr, "You cannot try to give a different type to an existing variable!", "TYPE MISTMATCH ERROR");
                return VALUE_ERROR;
            }
            result = symbol->type;
            break;
        }
        case EXPR_CALL:
        {
            Symbol* symbol = lookUpSymbol(checker, expr->objectCall.callee->variable.name, expr->objectCall.callee->variable.length);
            if (symbol == NULL || symbol->function == NULL)
            {
                typeError(checker, parser, expr, "The object you are calling must be an existing function or class instance.", "UNDEFINED ERROR");
                return VALUE_ERROR;
            }
            if (expr->objectCall.callee->type != EXPR_VARIABLE)
            {
                typeError(checker, parser, expr, "Callee must be a named function.", "TYPE ERROR");
                return VALUE_ERROR;
            }
            if (expr->objectCall.argCount != symbol->function->arity)
            {
                typeError(checker, parser, expr, "Function call parameters amount does not match function definition.", "UNDEFINED ERROR");
                return VALUE_ERROR;
            }

            for (int i = 0; i < symbol->function->arity; i++)
            {
                ValueType argType = checkExpression(checker, expr->objectCall.args[i], parser);
                if (symbol->function->params[i].type == VALUE_ANY) continue;
                if (symbol->function->params[i].type == VALUE_ANY_NUM)
                {
                    if (!isNumeric(argType))
                    {
                        typeError(checker, parser, expr, "Please only input numeric arguements (int, float, double)", "TYPE ERROR");
                        return VALUE_ERROR;
                    }
                    continue;
                }

                if (argType != symbol->function->params[i].type)
                {
                    typeError(checker, parser, expr, "An arguement of differing type was found in function call. Please double check function input.", "TYPE ERROR");
                    return VALUE_ERROR;
                }
            }

            result = symbol->function->returnType;
            break;
        }
        case EXPR_STATIC_ARRAY:
        {

            result = VALUE_ERROR;
            if (expr->staticArray.length == 0)
            {
                error("Cannot declare an empty array without a type.", "TYPE ERROR", parser);
                result =  VALUE_ERROR;
                break;
            }

            ValueType inferType = checkExpression(checker, expr->staticArray.values[0], parser);
            if (inferType == VALUE_ERROR) {result = VALUE_ERROR; break;}

            //check through all latter elements
            bool typeError = false;
            for (int i = 1; i < expr->staticArray.length; i++)
            {
                ValueType t = checkExpression(checker, expr->staticArray.values[i], parser);
                if (t != inferType || t == VALUE_ERROR)
                {
                    error("All array elements must be the same type.", "TYPE MISMATCH ERROR", parser);
                    typeError = true;
                }
            }
            if (!typeError)
            {
                expr->staticArray.type = inferType;
                result = toArrayType(inferType);
            }
            expr->staticArray.type = inferType;
            break;
        }
        case EXPR_GET_ARRAY_INDEX:
        {
            ValueType arrayType = checkExpression(checker, expr->getArray.left, parser);
            ValueType indexType = checkExpression(checker, expr->getArray.index, parser);

            if (indexType != VALUE_INT)
            {
                error("An index into an array must be of value type INT, please only use those.", "INDEX ERROR", parser);
                result = VALUE_ERROR;
                break;
            }
            result = toElementType(arrayType);
            break;
        }
        case EXPR_SET_ARRAY_INDEX:
        {
            ValueType arrayType = checkExpression(checker, expr->setArray.left, parser);
            ValueType indexType = checkExpression(checker, expr->setArray.index, parser);
            ValueType valueType = checkExpression(checker, expr->setArray.value, parser);

            if (indexType != VALUE_INT)
            {
                error("An index into an array must be of value type INT, please only use those.", "INDEX ERROR", parser);
                result = VALUE_ERROR;
                break;
            }
            if (valueType != toElementType(arrayType))
            {
                error("When assigning a new element in an array, the assigning type must match the array type.", "TYPE MISTMATCH ERROR", parser);
                result = VALUE_ERROR;
                break;
            }

            result = toElementType(arrayType);
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