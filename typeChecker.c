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
    checker->lastClassName = NULL;
    checker->lastClassNameLength = 0;
    checker->currentClassName = NULL;
    checker->currentClassNameLength = 0;
}



//-----------------Helper Functions--------------//
static bool isNumeric(ValueType type)
{
    return type == VALUE_INT || type == VALUE_DOUBLE || type == VALUE_FLOAT;
}
static bool isArray(ValueType type)
{
    return type == VALUE_BOOL_ARRAY || type == VALUE_DOUBLE_ARRAY || type == VALUE_FLOAT_ARRAY
    || type == VALUE_INT_ARRAY || type == VALUE_STRING_ARRAY || type == VALUE_EMPTY_ARRAY || type == VALUE_OBJECT_ARRAY;
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
static void registerNativeSymbol(TypeChecker* checker, const char* name, int length, ValueType returnType, ParamInfo* params, int paramCount, ASTparser* parser, struct Vm* vm)
{
    // create a dummy ObjFunction just to hold the signature
    ObjFunction* sig = calloc(1, sizeof(ObjFunction));
    sig->returnType = returnType;
    sig->arity = paramCount;
    for (int i = 0; i < paramCount; i++)
    {
        sig->params[i] = params[i];
    }
    addSymbol(checker, name, length, 0, returnType, sig, parser);
    checker->symbols[checker->varCount - 1].isTemp = true; //for freeing later so no memory leaks out
}
void registerIOSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{
    //print(ANY msg) -> void
    ParamInfo printParams[1];
    printParams[0].type = VALUE_ANY;
    printParams[0].name = "msg";
    printParams[0].length = 3;
    registerNativeSymbol(checker, "print", 5, VALUE_EMPTY, printParams, 1, parser, vm);

    //println(ANY msg) -> void
    ParamInfo printlnParams[1];
    printlnParams[0].type = VALUE_ANY;
    printlnParams[0].name = "msg";
    printlnParams[0].length = 3;
    registerNativeSymbol(checker, "println", 7, VALUE_EMPTY, printlnParams, 1, parser, vm);

    //intake(String/EMPTY prompt) -> string (the think user inputed)
    ParamInfo intakeParams[1];
    intakeParams[0].type = VALUE_ANY;
    intakeParams[0].name = "prompt";
    intakeParams[0].length = 6;
    intakeParams[0].isOptional = true;
    registerNativeSymbol(checker, "intake", 6, VALUE_STRING, intakeParams, 1, parser, vm);
}
void registerMathSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{

    ParamInfo mathParams[1];
    mathParams[0].type = VALUE_ANY_NUM;
    mathParams[0].name = "num";
    mathParams[0].length = 3;

    registerNativeSymbol(checker, "sin", 3, VALUE_DOUBLE, mathParams, 1, parser, vm);   //sin(double/float/int radians) -> double
    registerNativeSymbol(checker, "cos", 3, VALUE_DOUBLE, mathParams, 1, parser, vm);   //cos(double/float/int radians) -> double
    registerNativeSymbol(checker, "tan", 3, VALUE_DOUBLE, mathParams, 1, parser, vm);   //tan(double/float/int radians) -> double
    registerNativeSymbol(checker, "abs", 3, VALUE_DOUBLE, mathParams, 1, parser, vm);   //abs(double/float/int num) -> double
    registerNativeSymbol(checker, "ln", 2, VALUE_DOUBLE, mathParams, 1, parser, vm);    //ln(double/float/int baseE) -> double
    registerNativeSymbol(checker, "log10", 5, VALUE_DOUBLE, mathParams, 1, parser, vm); //log10(double/float/int base10) -> double
    registerNativeSymbol(checker, "log2", 4, VALUE_DOUBLE, mathParams, 1, parser, vm);  //log2(double/float/int base2) -> double
    registerNativeSymbol(checker, "sqrt", 4, VALUE_DOUBLE, mathParams, 1, parser, vm);  //sqrt(double/float/int nonNegtiveNum) -> double
    registerNativeSymbol(checker, "floor", 5, VALUE_DOUBLE, mathParams, 1, parser, vm); //floor(double/float/int num) -> double
    registerNativeSymbol(checker, "ceil", 4, VALUE_DOUBLE, mathParams, 1, parser, vm);  //ceil(double/float/int num) -> double
    registerNativeSymbol(checker, "expo", 4, VALUE_DOUBLE, mathParams, 1, parser, vm);  //expo(double/float/int toTheEPower) -> double

    ParamInfo mathParams2[2];
    mathParams2[0].type = VALUE_ANY_NUM;
    mathParams2[0].name = "num1";
    mathParams2[0].length = 4;
    mathParams2[1].type = VALUE_ANY_NUM;
    mathParams2[1].name = "num2";
    mathParams2[1].length = 4;
    registerNativeSymbol(checker, "pow", 3, VALUE_DOUBLE, mathParams2, 2, parser, vm);  //expo(double/float/int toTheEPower) -> double
}
void registerTimeSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{
    registerNativeSymbol(checker, "clock", 5, VALUE_DOUBLE, NULL, 0, parser, vm);
    registerNativeSymbol(checker, "time", 4, VALUE_DOUBLE, NULL, 0, parser, vm);
    registerNativeSymbol(checker, "dateString", 10, VALUE_STRING, NULL, 0, parser, vm);
    registerNativeSymbol(checker, "timeString", 10, VALUE_STRING, NULL, 0, parser, vm);

    ParamInfo sleepSymbols[1];
    sleepSymbols[0].name = "ms";
    sleepSymbols[0].length = 2;
    sleepSymbols[0].type = VALUE_ANY_NUM;
    registerNativeSymbol(checker, "sleep", 5, VALUE_DOUBLE, sleepSymbols, 1, parser, vm);
}
void registerFileIOSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{
    ParamInfo stringParam[1];
    stringParam[0].name = "path";
    stringParam[0].type = VALUE_STRING;
    stringParam[0].length = 4;
    registerNativeSymbol(checker, "readFile", 8, VALUE_STRING, stringParam, 1, parser, vm);
    registerNativeSymbol(checker, "fileExists", 10, VALUE_STRING, stringParam, 1, parser, vm);
    registerNativeSymbol(checker, "deleteFile", 10, VALUE_STRING, stringParam, 1, parser, vm);

    ParamInfo writeParam[2];
    writeParam[0].name = "path";
    writeParam[0].type = VALUE_STRING;
    writeParam[0].length = 4;
    writeParam[1].name = "content";
    writeParam[1].type = VALUE_STRING;
    writeParam[1].length = 7;
    registerNativeSymbol(checker, "writeFile", 9, VALUE_EMPTY, writeParam, 2, parser, vm);
    registerNativeSymbol(checker, "appendFile", 10, VALUE_EMPTY, writeParam, 2, parser, vm);

}
void registerTypeSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{
    //----float params---/
    ParamInfo floatParams[1];
    floatParams[0].name   = "floatVal";
    floatParams[0].length = 8;
    floatParams[0].type = VALUE_FLOAT;
    registerNativeSymbol(checker, "floatToInt", 10, VALUE_INT, floatParams, 1, parser, vm);
    registerNativeSymbol(checker, "floatToStr", 10, VALUE_STRING, floatParams, 1, parser, vm);
    registerNativeSymbol(checker, "floatToDouble", 13, VALUE_DOUBLE, floatParams, 1, parser, vm);

    //----Double params---/
    ParamInfo doubleParams[1];
    doubleParams[0].name   = "doubleVal";
    doubleParams[0].length = 9;
    doubleParams[0].type = VALUE_DOUBLE;
    registerNativeSymbol(checker, "doubleToInt", 11, VALUE_INT, doubleParams, 1, parser, vm);
    registerNativeSymbol(checker, "doubleToStr", 11, VALUE_STRING, doubleParams, 1, parser, vm);
    registerNativeSymbol(checker, "doubleToFloat", 13, VALUE_FLOAT, doubleParams, 1, parser, vm);

    //----Integer params---/
    ParamInfo intParams[1];
    intParams[0].name   = "intVal";
    intParams[0].length = 6;
    intParams[0].type = VALUE_INT;
    registerNativeSymbol(checker, "intToDouble", 11, VALUE_DOUBLE, intParams, 1, parser, vm);
    registerNativeSymbol(checker, "intToStr", 8, VALUE_STRING, intParams, 1, parser, vm);
    registerNativeSymbol(checker, "intToFloat", 10, VALUE_FLOAT, intParams, 1, parser, vm);

    //----String params---/
    ParamInfo stringParams[1];
    stringParams[0].name   = "stringVal";
    stringParams[0].length = 9;
    stringParams[0].type = VALUE_STRING;
    registerNativeSymbol(checker, "strToDouble", 11, VALUE_DOUBLE, stringParams, 1, parser, vm);
    registerNativeSymbol(checker, "strToInt", 8, VALUE_INT, stringParams, 1, parser, vm);
    registerNativeSymbol(checker, "strToBool", 9, VALUE_BOOL, stringParams, 1, parser, vm);
    registerNativeSymbol(checker, "strToFloat", 10, VALUE_FLOAT, stringParams, 1, parser, vm);

    //----Bool params---/
    ParamInfo boolParams[1];
    boolParams[0].name   = "boolVal";
    boolParams[0].length = 7;
    boolParams[0].type = VALUE_BOOL;
    registerNativeSymbol(checker, "boolToStr", 9, VALUE_STRING, boolParams, 1, parser, vm);

}
void registerHashMapSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{

}
void registerArrayListSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{

}
void registerStringSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{
    ParamInfo stringParams[1];
    stringParams[0].name   = "stringVal";
    stringParams[0].length = 9;
    stringParams[0].type = VALUE_STRING;

    registerNativeSymbol(checker, "strLength", 9, VALUE_INT, stringParams, 1, parser, vm);
    registerNativeSymbol(checker, "strToUpper", 10, VALUE_STRING, stringParams, 1, parser, vm);
    registerNativeSymbol(checker, "strToLower", 10, VALUE_STRING, stringParams, 1, parser, vm);

    ParamInfo twoStringParams[2];
    twoStringParams[0].name   = "stringVal";
    twoStringParams[0].length = 9;
    twoStringParams[0].type = VALUE_STRING;
    twoStringParams[1].name   = "stringVal2";
    twoStringParams[1].length = 10;
    twoStringParams[1].type = VALUE_STRING;
    registerNativeSymbol(checker, "strContains", 11, VALUE_BOOL, twoStringParams, 2, parser, vm);


    ParamInfo threeStringParams[3];
    threeStringParams[0].name   = "stringVal";
    threeStringParams[0].length = 9;
    threeStringParams[0].type = VALUE_STRING;
    threeStringParams[1].name   = "stringVal2";
    threeStringParams[1].length = 10;
    threeStringParams[1].type = VALUE_STRING;
    threeStringParams[2].name   = "stringVal3";
    threeStringParams[2].length = 10;
    threeStringParams[2].type = VALUE_STRING;
    registerNativeSymbol(checker, "strReplace", 10, VALUE_STRING, threeStringParams, 3, parser, vm);

    ParamInfo stringAndIntParams[3];
    stringAndIntParams[0].name   = "stringVal";
    stringAndIntParams[0].length = 9;
    stringAndIntParams[0].type = VALUE_STRING;
    stringAndIntParams[1].name   = "intVal1";
    stringAndIntParams[1].length = 7;
    stringAndIntParams[1].type = VALUE_INT;
    stringAndIntParams[2].name   = "intVal2";
    stringAndIntParams[2].length = 7;
    stringAndIntParams[2].type = VALUE_INT;
    registerNativeSymbol(checker, "strSlice", 8, VALUE_STRING, stringAndIntParams, 3, parser, vm);

}
void registerUtilsSymbols(TypeChecker* checker, struct ASTparser* parser, struct Vm* vm)
{
    ParamInfo arrayParams[1];
    arrayParams[0].type = VALUE_ANY_ARRAY;
    arrayParams[0].length = 5;
    arrayParams[0].name = "array";
    registerNativeSymbol(checker, "length", 6, VALUE_INT, arrayParams, 1, parser, vm);

    ParamInfo assertParams[2];
    assertParams[0].type = VALUE_BOOL;
    assertParams[0].length = 5;
    assertParams[0].name = "array";
    assertParams[1].type = VALUE_STRING;
    assertParams[1].length = 3;
    assertParams[1].name = "msg";
    assertParams[1].isOptional = true;
    registerNativeSymbol(checker, "assert", 6, VALUE_EMPTY, assertParams, 2, parser, vm);
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
ValueType checkClassField(TypeChecker* checker, Expr* expr, ASTparser* parser)
{
    Expr* callee;
    const char* fieldName;
    int fieldLength;

    //determine if its called in get or set field case
    if (expr->type == EXPR_GET_FIELD)
    {
        callee = expr->getField.callee;
        fieldName = expr->getField.fieldName;
        fieldLength = expr->getField.fieldLenght;
    }
    else //set field only other option
    {
        callee = expr->setField.callee;
        fieldName = expr->setField.fieldName;
        fieldLength = expr->setField.fieldLenght;
    }


    //check that the callee is a instance of a class
    ValueType objectType = checkExpression(checker, callee, parser);
    if (objectType != VALUE_INSTANCE)
    {
        typeError(checker, parser, expr, "You can only try and get a field value from a class instance.", "TYPE ERROR");
        return VALUE_ERROR;

    }

    //populate the knowledge of what class the call is coming from
    const char* className = NULL;
    int classNameLength = 0;
    if (callee->type == EXPR_VARIABLE)
    {
        //simple case of someInstance.value;
        Symbol* instanceSymbol = lookUpSymbol(checker, callee->variable.name, callee->variable.length);
        if (instanceSymbol != NULL)
        {
            //populate the class the var was gotten from;
            className = instanceSymbol->className;
            classNameLength = instanceSymbol->classNameLength;
        }
    }
    else
    {
        //chained call //someInstance1.someInstance2.value;
        className = checker->lastClassName;
        classNameLength = checker->lastClassNameLength;
    }

    //check to see if the name was actually populated
    if (className == NULL)
    {
        typeError(checker, parser, expr, "Unknown class type.", "TYPE ERROR");
        return VALUE_ERROR;
    }

    //get the last accessed class in the checker's metadata from earlier, check to see if it exists obviously
    Symbol* classSymbol = lookUpSymbol(checker, className, classNameLength);
    if (classSymbol == NULL)
    {
        typeError(checker, parser, expr, "Unknown or undefined class type was attempted to be called.", "TYPE ERROR");
        return VALUE_ERROR;
    }


    //now check to see if the field they want to access exists, and return that inner fields type
    for (int i = 0; i < classSymbol->fieldCount; i++)
    {
        if (classSymbol->fieldsInfo[i].length == fieldLength
            && memcmp(classSymbol->fieldsInfo[i].name, fieldName, fieldLength) == 0)
        {
            if (classSymbol->fieldsInfo[i].type == VALUE_INSTANCE)
            {
                checker->lastClassName = classSymbol->fieldsInfo[i].className;
                checker->lastClassNameLength = classSymbol->fieldsInfo[i].classNameLength;
            }
            return classSymbol->fieldsInfo[i].type;
        }
    }
    typeError(checker, parser, expr, "Undefined field.", "TYPE ERROR");
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
    checker->symbols[index].isTemp = false;
    checker->symbols[index].className = NULL;
    checker->symbols[index].classNameLength = 0;
    checker->symbols[index].fieldsInfo = NULL;
    checker->symbols[index].fieldCount = 0;
    checker->symbols[index].fieldCapacity = 0;
    checker->symbols[index].methodInfo = NULL;
    checker->symbols[index].methodCount = 0;
    checker->symbols[index].methodCapcaity = 0;
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
            //------------handling for actual method calls inside of classes------------//
            if (expr->objectCall.callee->type == EXPR_GET_FIELD)
            {
                //TODO: update this abhorrent code whenever I want to add the this keyword
                //check the type of the call expressions calle which is the get field expression which holds it's own
                //expr which is actually just the ValueInstance (very confusing I know)
                ValueType objectType = checkExpression(checker, expr->objectCall.callee->getField.callee, parser);
                if (objectType != VALUE_INSTANCE)
                {
                    typeError(checker, parser, expr, "Only class instances have methods that you are able to call.", "TYPE ERROR");
                    return VALUE_ERROR;
                }

                ///look up which class the method that you are trying to call belongs to, and then check out it's args
                const char* className = NULL;
                int classNameLength = 0;
                //populate the class name information
                if (expr->objectCall.callee->getField.callee->type == EXPR_VARIABLE) //basic class calls
                {
                    //get the instance call in look up, and then populated it with the stored class information there
                    Symbol* instanceSymbol = lookUpSymbol(checker, expr->objectCall.callee->getField.callee->variable.name, expr->objectCall.callee->getField.callee->variable.length);
                    if (instanceSymbol != NULL)
                    {
                        className = instanceSymbol->className;
                        classNameLength = instanceSymbol->classNameLength;
                    }
                }
                else if (expr->objectCall.callee->getField.callee->type == EXPR_THIS) //incase we are called inside of a class
                {
                    className = checker->currentClassName;
                    classNameLength = checker->currentClassNameLength;
                }
                else //more complicated in case of multiple calls
                {
                    //otherwise just get the last one stored in checker
                    className = checker->lastClassName;
                    classNameLength = checker->lastClassNameLength;
                }

                //error check for no class name being found
                if (className == NULL)
                {
                    typeError(checker, parser, expr, "Unknown class type called.", "TYPE ERROR");
                    result = VALUE_ERROR;
                    break;
                }

                //otherwise get the symbol for the class based on the gotten name so you can access the data there
                Symbol* classSymbol = lookUpSymbol(checker, className, classNameLength);
                if (classSymbol == NULL)
                {
                    typeError(checker, parser, expr, "Unknown class type called.", "TYPE ERROR");
                    result = VALUE_ERROR;
                    break;
                }

                //actually find the method inside of the class symbol as set up in functionDecalation in astcompiler.c
                const char* methodName = expr->objectCall.callee->getField.fieldName;
                int methodLength = expr->objectCall.callee->getField.fieldLenght;
                bool methodFound = false;
                for (int i = 0; i < classSymbol->methodCount; i++)
                {
                    //actually check to find the right method
                    if (classSymbol->methodInfo[i].length == methodLength && memcmp(classSymbol->methodInfo[i].name, methodName, methodLength) == 0)
                    {
                        //check the arg count
                        if (expr->objectCall.argCount != classSymbol->methodInfo[i].paramCount)
                        {
                            typeError(checker, parser, expr, "A different number of arguements was passed into class method call than defined in method's declaration.", "SYNTAX ERROR");
                            result = VALUE_ERROR;
                            methodFound = true;
                            break;
                        }

                        //check the argtypes of that method now
                        for (int j = 0; j < expr->objectCall.argCount; j++)
                        {
                            ValueType argType = checkExpression(checker, expr->objectCall.args[j], parser);
                            if (classSymbol->methodInfo[i].param[j].type != VALUE_ANY && argType != classSymbol->methodInfo[i].param[j].type)
                            {
                                typeError(checker, parser, expr, "Instance method call was found with an arguement different from declared parameter type", "TYPE MISMATCH ERROR");
                                result=VALUE_ERROR;
                                methodFound = true;
                                break;
                            }
                        }
                        result = classSymbol->methodInfo[i].returnType;
                        methodFound = true;
                        break;
                    }
                }
                if (!methodFound)
                {
                    typeError(checker, parser, expr, "Undefined method.", "TYPE ERROR");
                    result = VALUE_ERROR;
                }
                break;
            }


            //------------plain function handling-------------//
            Symbol* symbol = lookUpSymbol(checker, expr->objectCall.callee->variable.name, expr->objectCall.callee->variable.length);

            //basic null check
            if (symbol == NULL)
            {
                typeError(checker, parser, expr, "The object you are calling must be an existing function or class instance.", "UNDEFINED ERROR");
                return VALUE_ERROR;
            }

            //check for classes
            if (symbol->type == VALUE_CLASS)
            {
                if (expr->objectCall.argCount != 0)
                {
                    typeError(checker, parser, expr, "Class constructors don't take arguments yet.", "TYPE ERROR");
                    return VALUE_ERROR;
                }
                result = VALUE_INSTANCE;
                break;
            }

            //checking callee
            if (symbol->function == NULL)
            {
                typeError(checker, parser, expr, "The object you are calling must be an existing function or class instance.", "UNDEFINED ERROR");
                return VALUE_ERROR;
            }
            if (expr->objectCall.callee->type != EXPR_VARIABLE)
            {
                typeError(checker, parser, expr, "Callee must be a named function.", "TYPE ERROR");
                return VALUE_ERROR;
            }

            //checking arg counts
            int requiredCount = 0;
            for (int i = 0; i < symbol->function->arity; i++)
            {
                if (!symbol->function->params[i].isOptional) requiredCount++;
            }
            if (expr->objectCall.argCount < requiredCount || expr->objectCall.argCount > symbol->function->arity)
            {
                typeError(checker, parser, expr, "Function call parameters amount does not match function definition.", "UNDEFINED ERROR");
                return VALUE_ERROR;
            }

            //type checking args
            for (int i = 0; i < expr->objectCall.argCount; i++)
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
                if (symbol->function->params[i].type == VALUE_ANY_ARRAY)
                {
                    if (!isArray(argType))
                    {
                        typeError(checker, parser, expr, "Please input an array as an arguement into this function.", "TYPE ERROR");
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
        case EXPR_AND:
        {
            ValueType left = checkExpression(checker, expr->andExpr.left, parser);
            ValueType right = checkExpression(checker, expr->andExpr.right, parser);
            if (left == right && left == VALUE_BOOL)
            {
                result = VALUE_BOOL;
                break;
            }
            error("An expression involving '&&' must compare two boolean values please.", "TYPE MISMATCH ERROR", parser);
            result = VALUE_ERROR;
            break;
        }
        case EXPR_OR:
        {
            ValueType left = checkExpression(checker, expr->orExpr.left, parser);
            ValueType right = checkExpression(checker, expr->orExpr.right, parser);
            if (left == right && left == VALUE_BOOL)
            {
                result = VALUE_BOOL;
                break;
            }
            error("An expression involving '||' must compare two boolean values please.", "TYPE MISMATCH ERROR", parser);
            result = VALUE_ERROR;
            break;
        }
        case EXPR_GET_FIELD:
        {
            result = checkClassField(checker, expr, parser);
            break;
        }
        case EXPR_SET_FIELD:
        {
            ValueType classFieldType = checkClassField(checker, expr, parser);
            ValueType newValueType = checkExpression(checker, expr->setField.newValue, parser);

            if (classFieldType == newValueType && classFieldType != VALUE_ERROR)
            {
                result = newValueType;
                break;
            }
            typeError(checker, parser, expr, "Mismatch typing when attempting to assign a new value to a  class instance's field", "TYPE ERROR");
            result = VALUE_ERROR;
            break;

        }
        case EXPR_THIS:
        {
            //check to make sure that 'this' is used inside of a class context
            if (checker->currentClassName == NULL)
            {
                typeError(checker, parser, expr, "You may not use 'this' outside of a class method, there would be no 'this' to call.", "LOGIC ERROR");
                result = VALUE_ERROR;
                break;
            }
            //update lastClassName so that way chained calls using this would work as well
            checker->lastClassName = checker->currentClassName;
            checker->lastClassNameLength = checker->currentClassNameLength;
            result = VALUE_INSTANCE; //since this is basically the clas s itself
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
void freeTypeChecker(TypeChecker* checker)
{
    for (int i = 0; i< checker->varCount; i++)
    {
        if (checker->symbols[i].isTemp && checker->symbols[i].function != NULL)
        {
            free(checker->symbols[i].function);
            checker->symbols[i].function = NULL;
        }
        if (checker->symbols[i].fieldsInfo != NULL)
        {
            free(checker->symbols[i].fieldsInfo);
            checker->symbols[i].fieldsInfo = NULL;
        }
        if (checker->symbols[i].methodInfo != NULL)
        {
            free(checker->symbols[i].methodInfo);
            checker->symbols[i].methodInfo = NULL;
        }

    }
}
