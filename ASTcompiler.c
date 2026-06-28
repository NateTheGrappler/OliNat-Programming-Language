//
// Created by natang on 5/30/26.
//

#include "ASTcompiler.h"

#include "Expr.h"
#include "Bytecompiler.h"

//TODO: add in && and || operators for operators

//-------DECLARATIONS------------//
static void advance(ASTparser* parser);
static Expr* astExpression(ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* Array(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* number(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* boolean(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* string(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* unary(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* grouping(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* binary(bool canAssign, ASTparser* parser, AstCompiler* compiler,  Expr* left, Vm* vm);
static Expr* dot(bool canAssign, ASTparser* parser, AstCompiler* compiler,  Expr* left, Vm* vm);
static Expr* _or(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm);
static Expr* _and(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm);
static Expr* acesssArray(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm);
static Expr* variable(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* _this(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static Expr* functionCall(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm);
static void declaration(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm);
static void statement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm);
static void expressionStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm);

//------------------------------------------Structs holding all fun data-----------------------------------------------------//
//defines language grammer for expressions
typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // || / or
    PREC_AND,           // && / and
    PREC_EQUALITY,      // ==
    PREC_COMPARISION,   // >, <, >=, <=
    PREC_TERM,          // +, -
    PREC_FACTOR,        // *, /
    PREC_UNARY,         // -, !
    PREC_CALL,          // . ()
    PREC_PRIMARY        //literal values like strings or bools or doubles
} Precedence;

//The rules for the prat parser stuff, basically the brain for the compiler
//set up a function pointer and set up the different fillable fields for these functions
typedef Expr* (*PrefixFn)(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm);
typedef Expr* (*InfixFn)(bool canAssign, ASTparser* parser, AstCompiler* compiler,  Expr* left, Vm* vm);

typedef struct
{
    PrefixFn prefix;
    InfixFn infix;
    Precedence precedence;
} ParseRule;

ParseRule rules[] = {
  [T_LEFT_PAREN]    = {grouping,functionCall,PREC_CALL},
  [T_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [T_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
  [T_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [T_LEFT_BRACKET]   = {Array,acesssArray,PREC_CALL},
  [T_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [T_DOT]           = {NULL,      dot,   PREC_CALL},
  [T_MINUS]         = {unary,     binary,PREC_TERM},
  [T_PLUS]          = {NULL,     binary, PREC_TERM},
  [T_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [T_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [T_STAR]          = {NULL,     binary, PREC_FACTOR},
  [T_BANG]          = {unary,     NULL,  PREC_NONE},
  [T_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [T_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [T_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [T_GREATER]       = {NULL,     binary, PREC_COMPARISION},
  [T_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISION},
  [T_LESS]          = {NULL,     binary, PREC_COMPARISION},
  [T_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISION},
  [T_IDENTIFIER]    = {variable, NULL,   PREC_NONE},

    //TODO: check types
  [T_STRING]        = {NULL,     NULL,   PREC_NONE},
  [T_FLOAT]         = {NULL,     NULL,   PREC_NONE},
  [T_DOUBLE]        = {NULL,     NULL,   PREC_NONE},
  [T_INTEGER]       = {NULL,     NULL,   PREC_NONE},
  [T_BOOL]          = {NULL,     NULL,   PREC_NONE},
  [T_STRING_VAL]    = {string,   NULL,   PREC_NONE},
  [T_FLOAT_VAL]     = {number,   NULL,   PREC_NONE},
  [T_DOUBLE_VAL]    = {number,   NULL,   PREC_NONE},
  [T_INTEGER_VAL]   = {number,   NULL,   PREC_NONE},

  [T_AND]           = {NULL,     _and,   PREC_AND},
  [T_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [T_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [T_FALSE]         = {boolean,  NULL,   PREC_NONE},
  [T_FOR]           = {NULL,     NULL,   PREC_NONE},
  [T_FUN]           = {NULL,     NULL,   PREC_NONE},
  [T_IF]            = {NULL,     NULL,   PREC_NONE},
  [T_EMPTY]         = {NULL,     NULL,   PREC_NONE},
  [T_OR]            = {NULL,     _or,    PREC_OR},
  [T_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [T_PULLF]         = {NULL,     NULL,   PREC_NONE},
  [T_THIS]          = {_this,     NULL,   PREC_NONE},
  [T_TRUE]          = {boolean,  NULL,   PREC_NONE},
  [T_MAKE]          = {NULL,     NULL,   PREC_NONE},
  [T_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [T_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [T_EOF]           = {NULL,     NULL,   PREC_NONE},
};


//-------DECLARATIONS------------//
static Expr* parserPrecedence(Precedence precedence, ASTparser* parser, AstCompiler* compiler, Vm* vm);
static ParseRule* getRule(TokenType type);

//------------------------------------------Init functions-----------------------------------------------------//
void initAstCompiler(AstCompiler* compiler)
{
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->isTopLevel = false;
    compiler->isClassMethod = false; //not really using this function no more, but for the memories ill add this
}
void initParser(ASTparser* parser, const char* source)
{
    //set up scanner (owned by parser)
    initScanner(source, &parser->scanner);

    //init the parser fields;
    parser->hadError = false;
    parser->panicMode = false;
    advance(parser); //get the scanner goin
}


//------------------------------------------Error handling functions-----------------------------------------------------//

//main error handling function for compile time
void errorAt(Token* token, const char* message, const char* messageType, ASTparser* parser)
{
    if (parser->panicMode) return; //skip if error already detected

    fprintf(stderr, ":>>  %s -- [line %d", messageType, token->line);

    if (token->type == T_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == T_ERROR)
    {
        //nothing
    }
    else
    {
        //print out erroring code
        fprintf(stderr, " at '%.*s'", token->length, token->lexemeStart);
    }

    fprintf(stderr, ": %s]\n", message);
    parser->hadError = true;
    parser->panicMode = true;
}
//error at previous token
void error(const char* message, const char* messageType, ASTparser* parser)
{
    errorAt(&parser->previous, message, messageType, parser);
}
//error at current looking at token
void errorAtCurrent(const char* message, const char* messageType, ASTparser* parser)
{
    errorAt(&parser->current, message, messageType, parser);
}
//my personal favorite function, check for necessary token
void consume(TokenType type, const char* message, const char* messageType, ASTparser* parser)
{
    if (parser->current.type == type)
    {
        advance(parser);
        return;
    }
    errorAtCurrent(message, messageType, parser);
}


//------------------------------------------Parsing functions-----------------------------------------------------//
static void advance(ASTparser* parser)
{
    parser->previous = parser->current;

    //loop through each token until you find one that isnt an error
    //(Allows for panic mode)
    for (;;)
    {
        parser->current = scanToken(&parser->scanner);
        //printToken(parser->current, &parser->scanner);
        if (parser->current.type != T_ERROR) break;
        errorAtCurrent(parser->current.lexemeStart, "PARSER ERROR", parser);
    }
}
static bool check(TokenType type, ASTparser* parser)
{
    return parser->current.type == type;
}
static bool match(TokenType type, ASTparser* parser)
{
    if (!check(type, parser)) return false;
    advance(parser);
    return true;
}


//literal expression parsing
static Expr* variable(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    //save the name of the var
    const char* name = parser->previous.lexemeStart;
    int length = parser->previous.length;
    int line = parser->previous.line;

    if (match(T_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser, compiler, vm);
        return createVarAssignment(name, length, value, line, vm);
    }

    Expr* self = createVariable(name, length, line, vm); //helper for doing the fancy += syntax
    if (match(T_PLUS_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser, compiler, vm);
        Expr* binary = createBinary(self, value, "+", line, vm);
        return createVarAssignment(name, length, binary, line, vm);
    }
    if (match(T_MINUS_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser, compiler, vm);
        Expr* binary = createBinary(self, value, "-", line, vm);
        return createVarAssignment(name, length, binary, line, vm);
    }
    if (match(T_STAR_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser, compiler, vm);
        Expr* binary = createBinary(self, value, "*", line, vm);
        return createVarAssignment(name, length, binary, line, vm);
    }
    if (match(T_SLASH_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser, compiler, vm);
        Expr* binary = createBinary(self, value, "/", line, vm);
        return createVarAssignment(name, length, binary, line, vm);
    }
    if (match(T_PLUS_PLUS, parser) && canAssign)
    {
        Expr* value = createLiteralInt(1, line, vm);
        Expr* binary = createBinary(self, value, "+", line, vm);
        return createVarAssignment(name, length, binary, line, vm);
    }
    if (match(T_MINUS_MINUS, parser) && canAssign)
    {
        Expr* value = createLiteralInt(1, line, vm);
        Expr* binary = createBinary(self, value, "-", line, vm);
        return createVarAssignment(name, length, binary, line, vm);
    }
    return self;
}
static Expr* Array(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    int valueCount = 0;
    Expr** values = NULL;
    while (!check(T_RIGHT_BRACKET, parser) && !check(T_EOF, parser))
    {
        Expr** newValues = reallocate(values, valueCount * sizeof(Expr*), sizeof(Expr*) * (valueCount + 1), vm);
        values = newValues;
        values[valueCount++] = astExpression(parser, compiler, vm);
        if (check(T_RIGHT_BRACKET, parser)) break;
        consume(T_COMMA, "Please separate array elements with a ','.", "SYNTAX ERROR", parser);
    }
    consume(T_RIGHT_BRACKET, "Please finish your array declaration with ']'.", "SYNTAX ERROR", parser);

    return createStaticArray(values, valueCount, VALUE_ERROR, parser->previous.line, vm);
}
static Expr* acesssArray(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm)
{
    Expr* index = astExpression(parser, compiler, vm);
    consume(T_RIGHT_BRACKET, "Please finish array access calls with a ']'.", "SYNTAX ERROR", parser);

    if (match(T_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser, compiler, vm);
        return createArraySet(left, index, value, parser->previous.line, vm);
    }

    return createArrayGet(left, index, parser->previous.line, vm);

}
static Expr* string(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    //copy the string because the source code representation might still be needed
    int length = parser->previous.length - 2;
    const char* src = parser->previous.lexemeStart + 1;

    char* chars = ALLOCATE(char, length+1, vm);
    int outLen = 0;

    for (int i = 0; i < length; i++)
    {
        if (src[i] == '\\' && i+1 < length)
        {
            i++;
            switch (src[i])
            {
                case 'n': chars[outLen++]  = '\n'; break;
                case 't': chars[outLen++]  = '\t'; break;
                case 'r': chars[outLen++]  = '\r'; break;
                case '\\': chars[outLen++] = '\\'; break;
                case '"': chars[outLen++]  = '"';  break;
                default:
                    chars[outLen++] = '\\';
                    chars[outLen++] = src[i];
                    break;
            }
        }
        else
        {
            chars[outLen++] = src[i];
        }
    }

    chars[outLen] = '\0';

    return createLiteralString(chars, parser->previous.line, vm);
}
static Expr* boolean(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    switch (parser->previous.type)
    {
        case T_TRUE: return createLiteralBool(true, parser->previous.line, vm);
        case T_FALSE: return createLiteralBool(false, parser->previous.line, vm);
        default: return NULL; //unreachable (hopefully)
    }
}
static Expr* number(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    //handle all three possible numbers you can have
    switch (parser->previous.type)
    {
        case T_DOUBLE_VAL:
        {
            double value = strtod(parser->previous.lexemeStart, NULL);
            return createLiteralDouble(value, parser->previous.line, vm);
        }
        case T_FLOAT_VAL:
        {
            float value = strtof(parser->previous.lexemeStart, NULL);
            return createLiteralFloat(value, parser->previous.line, vm);
        }
        case T_INTEGER_VAL:
        {
            long value = strtol(parser->previous.lexemeStart, NULL, 10);
            return createLiteralInt((int)value, parser->previous.line, vm);
        }
        default:
            return NULL; //unreachable (hopefully)

    }
}
static Expr* unary(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    //get unary type
    TokenType operatorType = parser->previous.type;

    //recursive call to consume next literal
    Expr* right = parserPrecedence(PREC_UNARY, parser, compiler, vm);

    switch (operatorType)
    {
        case T_MINUS:
        {
            return createUnary('-', right, parser->previous.line, vm);
        }
        case T_BANG:
        {
            return createUnary('!', right, parser->previous.line, vm);
        }
        default: return NULL; //unreachable
    }
}
static Expr* grouping(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    Expr* expr = astExpression(parser, compiler, vm);
    consume(T_RIGHT_PAREN, "Please finish all parentheses with a ')'.", "GROUPING ERROR", parser);
    return expr;
}
static Expr* binary(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm)
{
    //compile the second literal and also check out the type of operation you got
    TokenType operatorType = parser->previous.type;
    ParseRule* rule = getRule(operatorType);
    Expr* right = parserPrecedence((Precedence)(rule->precedence+1), parser, compiler, vm);

    switch (operatorType)
    {
        case T_PLUS:          return createBinary(left, right, "+", parser->previous.line, vm);
        case T_MINUS:         return createBinary(left, right, "-", parser->previous.line, vm);
        case T_STAR:          return createBinary(left, right, "*", parser->previous.line, vm);
        case T_SLASH:         return createBinary(left, right, "/", parser->previous.line, vm);
        case T_BANG_EQUAL:    return createBinary(left, right, "!=", parser->previous.line, vm);
        case T_EQUAL_EQUAL:   return createBinary(left, right, "==", parser->previous.line, vm);
        case T_GREATER:       return createBinary(left, right, ">", parser->previous.line, vm);
        case T_GREATER_EQUAL: return createBinary(left, right, ">=", parser->previous.line, vm);
        case T_LESS:          return createBinary(left, right, "<", parser->previous.line, vm);
        case T_LESS_EQUAL:    return createBinary(left, right, "<=", parser->previous.line, vm);
    }
}
static Expr* _and(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm)
{
    Expr* right = parserPrecedence(PREC_AND, parser, compiler, vm);
    return createAnd(left, right, parser->previous.line, vm);
}
static Expr* _or(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm)
{
    Expr* right = parserPrecedence(PREC_AND, parser, compiler, vm);
    return createOr(left, right, parser->previous.line, vm);
}
static Expr* _this(bool canAssign, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    return createThisExpr(parser->previous.line, vm);
}
static Expr* dot(bool canAssign, ASTparser* parser, AstCompiler* compiler,  Expr* left, Vm* vm)
{
    consume(T_IDENTIFIER, "The dot operator must be calling some field or method from a class instance", "SYNTAX ERROR", parser);
    const char* fieldName = parser->previous.lexemeStart;
    int nameLength = parser->previous.length;
    int line = parser->previous.line;

    if (match(T_EQUAL, parser) && canAssign)
    {
        //you know there is an assignment call if there is an '='
        Expr* value = astExpression(parser, compiler, vm);
        return createSetField(left, value, fieldName, nameLength, line, vm);
    }

    //otherwise you know it is just a get call, sigh, also add in support for all the fancy syntax shortcuts
    if (canAssign)
    {
        if (match(T_PLUS_EQUAL, parser))
        {
            Expr* get = createGetField(left, fieldName, nameLength, line, vm);
            Expr* value = astExpression(parser, compiler, vm);
            Expr* binary = createBinary(get, value, "+", line, vm);
            Expr* leftCopy = ALLOCATE(Expr, 1, vm);
            *leftCopy = *left;
            return createSetField(leftCopy, binary, fieldName, nameLength, line, vm);
        }
        if (match(T_MINUS_EQUAL, parser))
        {
            Expr* get = createGetField(left, fieldName, nameLength, line, vm);
            Expr* value = astExpression(parser, compiler, vm);
            Expr* binary = createBinary(get, value, "-", line, vm);
            Expr* leftCopy = ALLOCATE(Expr, 1, vm);
            *leftCopy = *left;
            return createSetField(leftCopy, binary, fieldName, nameLength, line, vm);
        }
        if (match(T_STAR_EQUAL, parser))
        {
            Expr* get = createGetField(left, fieldName, nameLength, line, vm);
            Expr* value = astExpression(parser, compiler, vm);
            Expr* binary = createBinary(get, value, "*", line, vm);
            Expr* leftCopy = ALLOCATE(Expr, 1, vm);
            *leftCopy = *left;
            return createSetField(leftCopy, binary, fieldName, nameLength, line, vm);
        }
        if (match(T_SLASH_EQUAL, parser))
        {
            Expr* get = createGetField(left, fieldName, nameLength, line, vm);
            Expr* value = astExpression(parser, compiler, vm);
            Expr* binary = createBinary(get, value, "/", line, vm);
            Expr* leftCopy = ALLOCATE(Expr, 1, vm);
            *leftCopy = *left;
            return createSetField(leftCopy, binary, fieldName, nameLength, line, vm);
        }
        if (match(T_PLUS_PLUS, parser))
        {
            Expr* get = createGetField(left, fieldName, nameLength, line, vm);
            Expr* one = createLiteralInt(1, line, vm);
            Expr* binary = createBinary(get, one, "+", line, vm);
            //create a copy of left since it would get shared between binary and this and get freed twice
            Expr* leftCopy = ALLOCATE(Expr, 1, vm);
            *leftCopy = *left;
            return createSetField(leftCopy, binary, fieldName, nameLength, line, vm);
        }
        if (match(T_MINUS_MINUS, parser))
        {
            Expr* get = createGetField(left, fieldName, nameLength, line, vm);
            Expr* one = createLiteralInt(1, line, vm);
            Expr* binary = createBinary(get, one, "-", line, vm);
            //create a copy of left since it would get shared between binary and this and get freed twice
            Expr* leftCopy = ALLOCATE(Expr, 1, vm);
            *leftCopy = *left;
            return createSetField(leftCopy, binary, fieldName, nameLength, line, vm);
        }
    }
    //basic plane call
    return createGetField(left, fieldName, nameLength, line, vm);
}


//actual parser precedence stuff
static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}
static Expr* parserPrecedence(Precedence precedence, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    //advance to next token, get the rule for the literal value and then some
    //error catching for bad snytax
    advance(parser);
    PrefixFn prefixRule = getRule(parser->previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expected expression", "SYNTAX ERROR", parser);
        return NULL;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT; //for when i add variables
    Expr* left = prefixRule(precedence <= PREC_ASSIGNMENT, parser, compiler, vm);

    //consume the actual expression operator and what not, lead the recursive portion of
    //this setup
    while (precedence <= getRule(parser->current.type)->precedence)
    {
        advance(parser);
        InfixFn infixRule = getRule(parser->previous.type)->infix;
        left = infixRule(canAssign, parser, compiler, left, vm);
    }

    return left;
}
static Expr* astExpression(ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    return parserPrecedence(PREC_ASSIGNMENT, parser, compiler, vm);
}


//------Variable Items-------//
static ValueType getVarDeclarationType(ASTparser* parser, TypeChecker* checker)
{
    switch (parser->current.type)
    {
        case T_INTEGER:
        {
            consume(T_INTEGER, "Please declare the type of the variable you wish to create after 'make'.", "SYNTAX ERROR", parser);
            if (match(T_LEFT_BRACKET, parser))
            {
                consume(T_RIGHT_BRACKET, "Please make sure all '[' have a corresponding ']' in your function params.", "SYNTAX ERROR", parser);
                return VALUE_INT_ARRAY;
            }
            return VALUE_INT;
        }
        case T_FLOAT:
        {
            consume(T_FLOAT, "Please declare the type of the variable you wish to create after 'make'.", "SYNTAX ERROR", parser);
            if (match(T_LEFT_BRACKET, parser))
            {
                consume(T_RIGHT_BRACKET, "Please make sure all '[' have a corresponding ']' in your function params.", "SYNTAX ERROR", parser);
                return VALUE_FLOAT_ARRAY;
            }
            return VALUE_FLOAT;
        }
        case T_DOUBLE:
        {
            consume(T_DOUBLE, "Please declare the type of the variable you wish to create after 'make'.", "SYNTAX ERROR", parser);
            if (match(T_LEFT_BRACKET, parser))
            {
                consume(T_RIGHT_BRACKET, "Please make sure all '[' have a corresponding ']' in your function params.", "SYNTAX ERROR", parser);
                return VALUE_DOUBLE_ARRAY;
            }
            return VALUE_DOUBLE;
        }
        case T_STRING:
        {
            consume(T_STRING, "Please declare the type of the variable you wish to create after 'make'.", "SYNTAX ERROR", parser);
            if (match(T_LEFT_BRACKET, parser))
            {
                consume(T_RIGHT_BRACKET, "Please make sure all '[' have a corresponding ']' in your function params.", "SYNTAX ERROR", parser);
                return VALUE_STRING_ARRAY;
            }
            return VALUE_STRING;
        }
        case T_BOOL:
        {
            consume(T_BOOL, "Please declare the type of the variable you wish to create after 'make'.", "SYNTAX ERROR", parser);
            if (match(T_LEFT_BRACKET, parser))
            {
                consume(T_RIGHT_BRACKET, "Please make sure all '[' have a corresponding ']' in your function params.", "SYNTAX ERROR", parser);
                return VALUE_STRING_ARRAY;
            }
            return VALUE_BOOL;
        }
        case T_EMPTY:
        {
            consume(T_EMPTY, "Please declare the type of the variable you wish to create after 'make'.", "SYNTAX ERROR", parser);
            if (match(T_LEFT_BRACKET, parser))
            {
                consume(T_RIGHT_BRACKET, "Please make sure all '[' have a corresponding ']' in your function params.", "SYNTAX ERROR", parser);
                return VALUE_EMPTY_ARRAY;
            }
            return VALUE_EMPTY;
        }
        case T_IDENTIFIER:
        {
            consume(T_IDENTIFIER, "Expected a type after 'make'", "SYNTAX ERROR", parser);
            const char* name = parser->previous.lexemeStart;
            int length = parser->previous.length;

            Symbol* symbol = lookUpSymbol(checker, name, length);
            if (symbol!=NULL && symbol->type == VALUE_CLASS)
            {
                checker->lastClassName = name;
                checker->lastClassNameLength = length;
                return VALUE_INSTANCE;
            }

            error("Expected a type after 'make'", "SYNTAX ERROR", parser);
            return VALUE_ERROR;
        }
        default:
            //techically unreachable now with classes, but Ill keep it for old times sake
            error("Expected a type after 'make'", "SYNTAX ERROR", parser);
            return VALUE_ERROR;
    }
}
static void varDeclaration(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm, ValueType type)
{
    const char* name = parser->previous.lexemeStart;
    int nameLength = parser->previous.length;



    //get the equal and parse the expression
    consume(T_EQUAL, "Expected a '=' after you declare a new variable.", "SYNTAX ERROR", parser);
    Expr* varInitializer = astExpression(parser, compiler, vm);
    consume(T_SEMICOLON, "Expected ';' after you declare a new variable", "SYNTAX ERROR", parser);


    //check the type of the expression vs the declared type
    ValueType realType = checkExpression(checker, varInitializer, parser);
    if (realType != type)
    {
        error("A variable expression's type must be the same as it's declared type.", "TYPE MISMATCH ERROR", parser);
        freeExpr(varInitializer, vm); //fix possible leak on error
        return;
    }


    if (compiler->scopeDepth == 0)
    {
        //set the global to get defined here
        addSymbol(checker, name, nameLength, compiler->scopeDepth, type, NULL, parser);

        //check to see if the type is a class declaration somewhere, and update the metadata in the typechecker to know what class it is
        if (type == VALUE_INSTANCE)
        {
            Symbol* symbol = lookUpSymbol(checker, name, nameLength);
            symbol->className = checker->lastClassName;
            symbol->classNameLength = checker->lastClassNameLength;
        }

        compileBytecode(varInitializer, parser, &compiler->function->chunk, compiler, vm);
        emitDefineGlobal(name, nameLength, &compiler->function->chunk, parser, vm);
        freeExpr(varInitializer, vm);
    }
    else
    {
        //declare a new local and then actually parse the value
        Local* local = &compiler->locals[compiler->localCount++];
        local->name = name;
        local->length = nameLength;
        local->depth = compiler->scopeDepth;

        //the compiled value ends up on the stack and IS the local
        addSymbol(checker, name, nameLength, compiler->scopeDepth, type, NULL, parser);

        //check to see if the type is a class declaration somewhere, and update the metadata in the typechecker to know what class it is
        if (type == VALUE_INSTANCE)
        {
            Symbol* symbol = lookUpSymbol(checker, name, nameLength);
            symbol->className = checker->lastClassName;
            symbol->classNameLength = checker->lastClassNameLength;
        }

        compileBytecode(varInitializer, parser, &compiler->function->chunk, compiler, vm);
        freeExpr(varInitializer, vm);
    }

}

//block statements
static void beginScope(AstCompiler* compiler)
{
    compiler->scopeDepth++;
}
static void endScope(AstCompiler* compiler, TypeChecker* checker, Vm* vm, ASTparser* parser)
{
    compiler->scopeDepth--;

    //pop locals
    while (compiler->localCount > 0 && compiler->locals[compiler->localCount-1].depth > compiler->scopeDepth)
    {
        if (compiler->locals[compiler->localCount-1].isCaptured)
        {
            emitByte(OP_CLOSE_UPVALUE, &compiler->function->chunk, parser, vm);
        }
        else
        {
            emitByte(OP_POP, &compiler->function->chunk, parser, vm);
        }
        compiler->localCount--;
    }

    //pop in tandem with symbol table
    while (checker->varCount > 0 &&
       checker->symbols[checker->varCount - 1].depth > compiler->scopeDepth)
    {
        checker->varCount--;
    }
}
static void block(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    while (!check(T_RIGHT_BRACE, parser) && !check(T_EOF, parser))
    {
        declaration(parser, checker, compiler, vm);
    }

    consume(T_RIGHT_BRACE, "Each of your '{' must inturn have a matching '}', please.", "SYNTAX ERROR", parser);
}


//if statements
void patchJump(int offset, Chunk* currentChunk, ASTparser* parser)
{
    short jump = currentChunk->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over twin.", "MEMORY ERROR", parser);
    }

    currentChunk->byteCode[offset] = (jump >> 8) & 0xff;
    currentChunk->byteCode[offset+1] = jump & 0xff;
}
static void ifStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    consume(T_LEFT_PAREN, "Please supplement your if statement with a '(' after 'if'.", "SYNTAX ERROR", parser);
    Expr* condition = astExpression(parser, compiler, vm);
    consume(T_RIGHT_PAREN, "Please close your if statement's condition with ')'.","SYNTAX ERROR", parser);


    ValueType conditionType = checkExpression(checker, condition, parser);
    if (conditionType != VALUE_BOOL)
    {
        error("If condition must be a boolean.", "TYPE MISMATCH ERROR", parser);
    }
    compileBytecode(condition, parser, &compiler->function->chunk, compiler, vm);
    freeExpr(condition, vm); //forgot to free condition expr haha

    //emit a jump instruction with then two supplementary bytes to store how large the jump is
    short jumpIndex = emitJump(OP_JUMP_IF_FALSE, &compiler->function->chunk, parser, vm);
    emitByte(OP_POP, &compiler->function->chunk, parser, vm);         //get condition value off stack TODO: see if this causes issues
    statement(parser, checker, compiler, vm);     //parse block

    short elseJump = emitJump(OP_JUMP, &compiler->function->chunk, parser, vm); //always jump if found

    patchJump(jumpIndex, &compiler->function->chunk, parser);
    emitByte(OP_POP, &compiler->function->chunk, parser, vm);


    if (match(T_ELSE, parser))
    {
        statement(parser, checker, compiler, vm);     //parse else block
    }
    patchJump(elseJump, &compiler->function->chunk, parser);

}


//while loops and for loops
static void emitLoop(int loopStart, ASTparser* parser, AstCompiler* compiler, Vm* vm)
{
    emitByte(OP_LOOP, &compiler->function->chunk, parser, vm);

    int offset = compiler->function->chunk.count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body is too large.", "MEMORY ERROR", parser);

    emitByte(((offset >> 8) & 0xff), &compiler->function->chunk, parser, vm);
    emitByte((offset  & 0xff), &compiler->function->chunk, parser, vm);
}
static void whileStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    int loopStart = compiler->function->chunk.count;

    consume(T_LEFT_PAREN, "Please supplement your while statement with a '(' after 'while'.", "SYNTAX ERROR", parser);
    Expr* condition = astExpression(parser, compiler, vm);
    consume(T_RIGHT_PAREN, "Please close your while statement's condition with ')'.", "SYNTAX ERROR", parser);


    ValueType conditionType = checkExpression(checker, condition, parser);
    if (conditionType != VALUE_BOOL)
    {
        error("While condition must be a boolean.", "TYPE MISMATCH", parser);
    }
    compileBytecode(condition, parser, &compiler->function->chunk, compiler, vm);
    freeExpr(condition, vm); //forgot to free condition expr haha

    int exitJump = emitJump(OP_JUMP_IF_FALSE, &compiler->function->chunk, parser, vm);
    emitByte(OP_POP, &compiler->function->chunk, parser, vm);
    statement(parser, checker, compiler, vm);
    emitLoop(loopStart, parser, compiler, vm);

    patchJump(exitJump, &compiler->function->chunk, parser);
    emitByte(OP_POP, &compiler->function->chunk, parser, vm);
}
static void forStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    beginScope(compiler);
    consume(T_LEFT_PAREN, "Please use a '(' character after the for keyword.", "SYNTAX ERROR", parser);

    if (match(T_SEMICOLON, parser))
    {
        //no
        //initializer, infinite loop
    }
    else if (match(T_MAKE, parser))
    {
        ValueType type = getVarDeclarationType(parser, checker);
        consume(T_IDENTIFIER, "You must name your variables and functions, they get sad if you dont.", "SYNTAX ERROR", parser);
        varDeclaration(parser, checker, compiler, vm, type);
    }
    else
    {
        expressionStatement(parser, checker, compiler, vm);
    }

    int loopStart = compiler->function->chunk.count;

    int exitJump = -1;
    if (!match(T_SEMICOLON, parser))
    {
        //calculate the actual condition
        Expr* expr = astExpression(parser, compiler, vm);
        ValueType type = checkExpression(checker, expr, parser);
        if (type != VALUE_BOOL) { error("A for loops condition must evaluate to a boolean expression.", "SYNTAX ERROR", parser); }

        compileBytecode(expr, parser, &compiler->function->chunk, compiler, vm);
        consume(T_SEMICOLON, "The code expects ';' after a for loop conditional.", "SYNTAX ERROR", parser);
        freeExpr(expr, vm); //forgot to free condition expr haha

        exitJump = emitJump(OP_JUMP_IF_FALSE, &compiler->function->chunk, parser, vm);
        emitByte(OP_POP, &compiler->function->chunk, parser, vm);

    }

    int bodyJump = -1;
    int incrementStart = -1;
    if (!match(T_RIGHT_PAREN, parser))
    {
        //evaluate the expression changing the loop
        bodyJump = emitJump(OP_JUMP, &compiler->function->chunk, parser, vm);
        incrementStart = compiler->function->chunk.count;

        Expr* expr = astExpression(parser, compiler, vm);
        compileBytecode(expr,parser, &compiler->function->chunk, compiler, vm);
        freeExpr(expr, vm); //forgot to free condition expr haha

        emitByte(OP_POP, &compiler->function->chunk, parser, vm);
        consume(T_RIGHT_PAREN, "Expect ')' after for loop clauses", "SYNTAX ERROR", parser);

        emitLoop(loopStart, parser, compiler, vm);
        loopStart = incrementStart;
        patchJump(bodyJump, &compiler->function->chunk, parser);
    }
    else
    {
        consume(T_RIGHT_PAREN, "Expect ')' after for clauses", "SYNTAX ERROR", parser);
    }

    statement(parser, checker, compiler, vm);
    emitLoop(loopStart, parser, compiler, vm);

    if (exitJump != -1)
    {
        patchJump(exitJump, &compiler->function->chunk, parser);
        emitByte(OP_POP, &compiler->function->chunk, parser, vm);
    }

    endScope(compiler, checker, vm, parser);
}

//functions
int addUpValue(AstCompiler* compiler, int index, bool isLocal)
{
    int count = compiler->function->upValueCount;
    for (int i = 0; i < count; i++)
    {
        //check to see if it is already captured, and then just return that
        if (compiler->upvalues[i].index == index && compiler->upvalues[i].isLocal == isLocal)
        {
            return i;
        }
    }

    //set up the new upval at that count
    compiler->upvalues[count].index = index;
    compiler->upvalues[count].isLocal = isLocal;
    return compiler->function->upValueCount++;
}
int resolveUpvalue(AstCompiler* compiler, const char* name, int length)
{
    if (compiler->enclosing == NULL) return -1;

    //look for that local in the enclosing scope, searching from top down
    for (int i = compiler->enclosing->localCount - 1; i >= 0; i--)
    {
        //get the local in said scope, and compare it to see if it matches
        Local* local = &compiler->enclosing->locals[i];
        if (local->length == length && memcmp(local->name, name, length) == 0)
        {
            local->isCaptured = true;
            return addUpValue(compiler, i, true);
        }
    }

    int upvalue = resolveUpvalue(compiler->enclosing, name, length);
    if (upvalue != -1) return addUpValue(compiler, upvalue, false);
    return -1;

}
static void initFunctionCompiler(AstCompiler* newCompiler, AstCompiler* enclosing, const char* name, int nameLength, ValueType returnType, bool isTopLevel, bool isMethod, Vm* vm, TypeChecker* checker, ASTparser* parser)
{
    memset(newCompiler, 0, sizeof(AstCompiler)); //zero out the compiler so it doesnt read garbage memory for an inner function and just die

    //zero out the compilers inner crap for locals
    newCompiler->isClassMethod = isMethod;
    newCompiler->localCount = 1;
    newCompiler->scopeDepth = (enclosing == NULL) ? 0 : 1;
    newCompiler->enclosing = enclosing;
    newCompiler->isTopLevel = isTopLevel;

    if (newCompiler->isClassMethod)
    {
        //tke up slot 0 for the 'this' keyword in the locals array instead of having it be the closure as normal
        //at least for methods
        Local* local = &newCompiler->locals[0];
        local->name = "this";
        local->length = 4;
        local->depth = 1;
    }

    //create the function compiler will fill and add to symbol table for recursive calls
    newCompiler->function = newFunction(name, nameLength, returnType, vm);
    push(vm, CREATE_OBJECT_VAL((Obj*)newCompiler->function)); //push onto stack to protect from gc

    if (!isTopLevel && !isMethod) { addSymbol(checker, name, nameLength, newCompiler->scopeDepth, returnType, newCompiler->function, parser); }

}
static ObjFunction* endFunctionCompiler(AstCompiler* compiler, ASTparser* parser, Vm* vm)
{
    ObjFunction* function = compiler->function;
    if (function->returnType != VALUE_EMPTY)
    {
        emitByte(OP_MISSING_RETURN, &function->chunk, parser, vm);
    }
    emitReturn(&function->chunk, parser, vm);
    return function;
}
static void functionDeclaration(ASTparser* parser, TypeChecker* checker,AstCompiler* compiler, Vm* vm, ValueType type, bool isMethod)
{
    //function name

    const char* name = parser->previous.lexemeStart;
    int nameLength = parser->previous.length;

    AstCompiler newCompiler;
    initFunctionCompiler(&newCompiler, compiler, name, nameLength, type, false, isMethod, vm, checker, parser);

    //check to see if the function is a constructor of a class object
    if (isMethod && checker->currentClassName != NULL && strncmp(name, checker->currentClassName, nameLength) == 0)
    {
        newCompiler.function->isConstructor = true;
    }

    //params
    consume(T_LEFT_PAREN, "Expected '(' after function name.", "SYNTAX ERROR", parser);
    while (!check(T_RIGHT_PAREN, parser) && !check(T_EOF, parser))
    {
        ValueType paramType = getVarDeclarationType(parser, checker);
        consume(T_IDENTIFIER, "A parameter is expected after a type definition in your function", "SYNTAX ERROR", parser);

        const char* paramName = parser->previous.lexemeStart;
        int paramLength = parser->previous.length;

        if (newCompiler.function->arity >= MAX_PARAMS)
        {
            error("Your function can only have a max of 255 params.", "MEMORY ERROR", parser);
        }
        else
        {
            ParamInfo* param = &newCompiler.function->params[newCompiler.function->arity++];
            param->type = paramType;
            param->name = paramName;
            param->length = paramLength;
        }
        //add in param as local on bottom of function's frame
        Local* local = &newCompiler.locals[newCompiler.localCount++];
        local->name = paramName;
        local->length = paramLength;
        local->depth = 1;

        addSymbol(checker, paramName, paramLength, 1, paramType, NULL, parser);

        if (check(T_RIGHT_PAREN, parser)) break;
        consume(T_COMMA, "Please seperate all function parameters with a ','.", "SYNTAX ERROR", parser);
    }
    consume(T_RIGHT_PAREN, "Expect ')' after you functions parameters.", "SYNTAX ERROR", parser);

    //function body
    consume(T_LEFT_BRACE, "Expected '{' after a function declaration", "SYNTAX ERROR", parser);
    block(parser, checker, &newCompiler, vm);

    //creating and passing on
    ObjFunction* function = endFunctionCompiler(&newCompiler, parser, vm);
    pop(vm); //pop function off vm for gc

    Symbol* existingSymbol = lookUpSymbol(checker, name, nameLength);
    if (existingSymbol) { existingSymbol->function = function; }  // Replace placeholder with real function

    emitConstant(CREATE_OBJECT_VAL((Obj*)function), &compiler->function->chunk, parser, vm); //create function object on stack
    emitByte(OP_CLOSURE, &compiler->function->chunk, parser, vm);                                //tell it to start the closure

    //emit each one of the upvalues as well
    for (int i = 0; i < function->upValueCount; i++)
    {
        emitByte(newCompiler.upvalues[i].isLocal ? 1 : 0, &compiler->function->chunk, parser, vm);
        emitByte(newCompiler.upvalues[i].index, &compiler->function->chunk, parser, vm);
    }

    if (compiler->scopeDepth > 0) //emit the function as a local only if it's nested
    {
        Local* local = &compiler->locals[compiler->localCount++];
        local->name = name;
        local->length = nameLength;
        local->depth = compiler->scopeDepth;
        local->isCaptured = false;
    }
    else
    {
        if (!isMethod)
        {
            emitDefineGlobal(name, nameLength, &compiler->function->chunk, parser, vm); //after resolving the closure and function, then define it as a new global var
        }
    }
}
static Expr* functionCall(bool canAssign, ASTparser* parser, AstCompiler* compiler, Expr* left, Vm* vm)
{
    int argCount = 0;
    Expr** args = NULL;
    while (!check(T_RIGHT_PAREN, parser) && !check(T_EOF, parser))
    {
        Expr** newArgs = reallocate(args, sizeof(Expr*) * argCount, sizeof(Expr*) * (argCount + 1), vm);
        args = newArgs;
        args[argCount++] = astExpression(parser, compiler, vm);
        if (check(T_RIGHT_PAREN, parser)) break;
        consume(T_COMMA, "Please seperate all function parameters with a ','.", "SYNTAX ERROR", parser);
    }
    consume(T_RIGHT_PAREN, "Please end all function calls with a ')'.", "SYNTAX ERROR", parser);


    return createCall(left, args, argCount, parser->previous.line, vm);
}
static void returnStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    //TODO: add in possible type promotion in return functions

    if (compiler->isTopLevel)
    {
        error("You cannot have 'return' in top level code.", "LOGICAL ERROR", parser);
    }

    if (!check(T_SEMICOLON, parser))
    {
        //Non void function returns
        Expr* returnExpr = astExpression(parser, compiler, vm);
        if (checkExpression(checker, returnExpr, parser) != compiler->function->returnType)
        {
            error("A return statement should match the type of it's parent function.", "TYPE ERROR", parser);
        }
        compileBytecode(returnExpr, parser, &compiler->function->chunk, compiler, vm);
        freeExpr(returnExpr, vm);
    }
    else
    {
        //void function returns
        if (compiler->function->returnType != VALUE_EMPTY)
        {
            error("Functions without the return type 'empty' must return a value.", "TYPE ERROR", parser);
        }
        emitByte(OP_CONSTANT, &compiler->function->chunk, parser, vm); //dummy value for popping in OP_RETURN

    }

    consume(T_SEMICOLON, "Please finish all return statements with a ';'.", "SYNTAX ERROR", parser);
    emitByte(OP_RETURN, &compiler->function->chunk, parser, vm);
}

//native stdlib stuff
static void nativeFunction(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    //check the type of native
    consume(T_IDENTIFIER, "You have to list the name of the library you wish to include.", "SYNTAX ERROR", parser);
    const char* library = parser->previous.lexemeStart;
    int length = parser->previous.length;

    if (strncmp(library, "stdlib", length) == 0)
    {
        registerIONatives(vm);
        registerMathNatives(vm);
        registerTimeNatives(vm);
        registerFileIONatives(vm);
        registerTypeNatives(vm);
        registerUtilsNatives(vm);
        registerStringNatives(vm);

        registerIOSymbols(checker, parser, vm);
        registerMathSymbols(checker, parser, vm);
        registerTimeSymbols(checker, parser, vm);
        registerFileIOSymbols(checker, parser, vm);
        registerTypeSymbols(checker, parser, vm);
        registerUtilsSymbols(checker, parser, vm);
        registerStringSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "io", length) == 0) //print & output functions
    {
        registerIONatives(vm);
        registerIOSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "math", length) == 0) //extra math functions
    {
        registerMathNatives(vm);
        registerMathSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "chronos", length) == 0) //time like clock and date
    {
        registerTimeNatives(vm);
        registerTimeSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "fileIO", length)== 0) //file reading, writting, creation, deletion
    {
        registerFileIONatives(vm);
        registerFileIOSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "types", length)== 0) //converstions between types
    {
        registerTypeNatives(vm);
        registerTypeSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "Hashmap", length) == 0)
    {
        registerHashMapNatives(vm);
        registerHashMapSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "ArrayList", length) == 0)
    {
        registerArrayListNatives(vm);
        registerArrayListSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "Strings", length) == 0)
    {
        registerStringNatives(vm);
        registerStringSymbols(checker, parser, vm);
    }
    else if (strncmp(library, "utils", length) == 0) //stuff like len() and maybe some sorts idk
    {
        registerUtilsNatives(vm);
        registerUtilsSymbols(checker, parser, vm);
    }
    else
    {
        error("Unknown library name.", "IMPORT ERROR", parser);
    }

}

//Basic statements
static void expressionStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    //compile and check expression, then compile again to bytecode
    Expr* expr = astExpression(parser, compiler, vm);
    ValueType type = checkExpression(checker, expr, parser);

    if (type != VALUE_ERROR)
    {
        compileBytecode(expr, parser, &compiler->function->chunk, compiler, vm);
        emitByte(OP_POP, &compiler->function->chunk, parser, vm);
    }

    //consume the ; and free memory
    consume(T_SEMICOLON, "Please end all of your expressions with a ';'!", "SYNTAX ERROR", parser);
    freeExpr(expr, vm);
}
static void statement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    if (match(T_LEFT_BRACE, parser))
    {
        beginScope(compiler);
        block(parser, checker, compiler, vm);
        endScope(compiler, checker, vm, parser);
    }
    else if (match(T_IF, parser))
    {
        ifStatement(parser, checker, compiler, vm);
    }
    else if (match(T_WHILE, parser))
    {
        whileStatement(parser, checker, compiler, vm);
    }
    else if (match(T_FOR, parser))
    {
        forStatement(parser, checker, compiler, vm);
    }
    else if (match(T_RETURN, parser))
    {
        returnStatement(parser, checker, compiler, vm);
    }
    else
    {
        expressionStatement(parser, checker, compiler, vm);
    }
}

//classes
static void classDeclaration(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    //consume name
    consume(T_IDENTIFIER, "Please name your classes, they get sad if you don't.", "SYNTAX ERROR", parser);
    const char* name = parser->previous.lexemeStart;
    int nameLength = parser->previous.length;

    //add into symbol table, if it hasnt already gotten added in the first look through
    Symbol* existing = lookUpSymbol(checker, name, nameLength);
    if(existing == NULL)
    {
        addSymbol(checker, name, nameLength, compiler->scopeDepth, VALUE_CLASS, NULL, parser);
    }

    //set up current class context in checker so 'this' can be typechecked inside methods
    checker->currentClassName = name;
    checker->currentClassNameLength = nameLength;

    //emit onto the stack (OP_CLASS CODE, INDEX FOR NAME)
    emitByte(OP_CLASS, &compiler->function->chunk, parser, vm);
    ObjString* nameString = copyString(name, nameLength, vm);
    uint8_t nameIndex = addConstant(&compiler->function->chunk, CREATE_OBJECT_VAL((Obj*)nameString), vm);
    emitByte(nameIndex, &compiler->function->chunk, parser, vm);


    consume(T_LEFT_BRACE, "A class body was expected after a new class declaration, please use '{'.", "SYNTAX ERROR", parser);
    while (!check(T_RIGHT_BRACE, parser) && !check(T_EOF, parser))
    {
        if (match(T_MAKE, parser))
        {
            //consume the identifer
            ValueType type = getVarDeclarationType(parser, checker);
            consume(T_IDENTIFIER, "Even inner class methods or fields have feelings, have some heart and give them a name.", "SYNTAX ERROR", parser);
            const char* name = parser->previous.lexemeStart;
            int nameLength = parser->previous.length;

            if (check(T_LEFT_PAREN, parser))
            {
                 //method dispatch the call to append a new method to the class
                functionDeclaration(parser, checker, compiler, vm, type, true);

                //after function gets declared and is on top of stack in bytecode, you add in the class method opcode to
                //make sure that that closure gets attached over to the class
                ObjString* methodName = copyString(name, nameLength, vm);
                uint8_t nameIndex = addConstant(&compiler->function->chunk, CREATE_OBJECT_VAL((Obj*)methodName), vm);

                //check for constructor
                if (strncmp(name, checker->currentClassName, checker->currentClassNameLength) == 0)
                {
                    //constructor has been found
                    emitByte(OP_CONSTRUCTOR, &compiler->function->chunk, parser, vm);
                }
                else
                {
                    //regular class method
                    emitByte(OP_CLASS_METHOD, &compiler->function->chunk, parser, vm);
                    emitByte(nameIndex, &compiler->function->chunk, parser, vm);
                }
            }
            else
            {
                //its just a field so store it's metadata

                Expr* initializer = NULL;
                if (match(T_EQUAL, parser)) //initialized with value
                {
                    initializer = astExpression(parser, compiler, vm);
                }
                consume(T_SEMICOLON, "Expected a ';' after a field declaration in class body.", "SYNTAX ERROR", parser);

                if (initializer != NULL)
                {
                    ValueType exprType = checkExpression(checker, initializer, parser);
                    if (exprType == type)
                    {
                        //compile the field
                        compileBytecode(initializer, parser, &compiler->function->chunk, compiler, vm);
                        freeExpr(initializer, vm);
                    }
                    else
                    {
                        //error if its the wrong type
                        error("Please make sure inner class field declarations match their explict type.", "TYPE ERROR", parser);
                        freeExpr(initializer, vm);
                    }
                }
                else
                {
                    //set up some default value based on the type if nothing was declared
                    emitByte(OP_FIELD_DEFAULT, &compiler->function->chunk, parser, vm);
                    emitByte((uint8_t)type, &compiler->function->chunk, parser, vm);
                }

                //compile the name as well
                ObjString* fieldName = copyString(name, nameLength, vm);
                uint8_t nameIndex = addConstant(&compiler->function->chunk, CREATE_OBJECT_VAL((Obj*)fieldName), vm);

                emitByte(OP_CLASS_FIELD, &compiler->function->chunk, parser, vm); //output the field instruction
                emitByte(nameIndex, &compiler->function->chunk, parser, vm);      //output index for name rightafter
                emitByte((uint8_t)type, &compiler->function->chunk, parser, vm);  //output the TYPE of the field for book keeping inside class
            }
        }
        else
        {
            error("Please only include either field or method declarations inside of a class declaration body.", "SYNTAX ERROR", parser);
            advance(parser);
        }
    }
    consume(T_RIGHT_BRACE, "A complete class body was expected after a new class declaration, please use '}'.", "SYNTAX ERROR", parser);


    //clear the typechecker context after parsing the body is done for a class
    checker->currentClassName = NULL;
    checker->currentClassNameLength = 0;

    //define as global so it can be called
    emitDefineGlobal(name, nameLength, &compiler->function->chunk, parser, vm);
}


//newer stuff for the statements and variables
static void declaration(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    if (match(T_MAKE, parser))
    {
        //get / consume the declaraed type, and then name
        ValueType type = getVarDeclarationType(parser, checker);

        //check for [] for arrays
        if (match(T_LEFT_BRACKET, parser))
        {
            consume(T_RIGHT_BRACKET, "Please match all '[' with a ']',", "SYNTAX ERROR", parser);
            type = toArrayType(type);
        }

        //get var name
        consume(T_IDENTIFIER, "You must name your variables and functions, they get sad if you dont.", "SYNTAX ERROR", parser);


        if (check(T_LEFT_PAREN, parser))
        {
            functionDeclaration(parser, checker, compiler, vm, type, false);
        }
        else
        {
            varDeclaration(parser, checker, compiler, vm, type);
        }
    }
    else if (match(T_HASH_PULLF, parser))
    {
        nativeFunction(parser, checker, compiler, vm);
    }
    else if (match(T_CLASS, parser))
    {
        classDeclaration(parser, checker, compiler, vm);
    }
    else
    {
        statement(parser, checker, compiler, vm);
    }
}
static void declareFunction(ASTparser* parser, TypeChecker* checker, Vm* vm)
{
    if (match(T_MAKE, parser))
    {
        ValueType type = getVarDeclarationType(parser, checker);
        if (check(T_IDENTIFIER, parser)) {consume(T_IDENTIFIER, "You must name your variables and functions, they get sad if you dont.", "SYNTAX ERROR", parser);}
        else if (check(T_LEFT_BRACKET, parser))
        {
            consume(T_LEFT_BRACKET, "", "SYNTAX ERROR", parser);
            consume(T_RIGHT_BRACKET, "Please match all '[' with a corresponding ']'.", "SYNTAX ERROR", parser);
            consume(T_IDENTIFIER, "You must name your variables and functions, they get sad if you dont.", "SYNTAX ERROR", parser);
        }
        if (check(T_LEFT_PAREN, parser))
        {
            const char* name = parser->previous.lexemeStart;
            int nameLength = parser->previous.length;

            //temp function
            ObjFunction* functionDeclaration = newFunction(name, nameLength, type, vm);
            push(vm, CREATE_OBJECT_VAL((Obj*)functionDeclaration)); //push for gc sake
            functionDeclaration->arity = 0;
            functionDeclaration->returnType = type;

            consume(T_LEFT_PAREN, "Expected '(' after function name.", "SYNTAX ERROR", parser);

            while (!check(T_RIGHT_PAREN, parser) && !check(T_EOF, parser))
            {
                ValueType paramType = getVarDeclarationType(parser, checker);
                consume(T_IDENTIFIER, "A parameter is expected after a type definition in your function", "SYNTAX ERROR", parser);

                const char* paramName = parser->previous.lexemeStart;
                int paramLength = parser->previous.length;

                if (functionDeclaration->arity >= MAX_PARAMS)
                {
                    error("Your function can only have a max of 255 params.", "MEMORY ERROR", parser);
                }
                else
                {
                    ParamInfo* param = &functionDeclaration->params[functionDeclaration->arity++];
                    param->type = paramType;
                    param->name = paramName;
                    param->length = paramLength;
                }


                if (check(T_RIGHT_PAREN, parser)) break;
                consume(T_COMMA, "Please seperate all function parameters with a ','.", "SYNTAX ERROR", parser);
            }
            consume(T_RIGHT_PAREN, "Expect ')' after you functions parameters.", "SYNTAX ERROR", parser);

            //add to func symbol table
            addSymbol(checker, name, nameLength, 0, type, functionDeclaration, parser);
            pop(vm); //get off of stack
            //skip over the body
            if (check(T_LEFT_BRACE, parser))
            {
                int braceCount = 1;
                consume(T_LEFT_BRACE, "", "", parser);
                while (braceCount > 0 && !check(T_EOF, parser))
                {
                    if (match(T_LEFT_BRACE, parser)) braceCount++;
                    else if (match(T_RIGHT_BRACE, parser)) braceCount--;
                    else advance(parser);
                }
            }
            else if (check(T_SEMICOLON, parser))
            {
                consume(T_SEMICOLON, "", "", parser); // Forward declaration
            }
        }
    }
    else if (match(T_HASH_PULLF, parser))
    {
        consume(T_IDENTIFIER, "You have to list the name of the library you wish to include.", "SYNTAX ERROR", parser);
        const char* library = parser->previous.lexemeStart;
        int length = parser->previous.length;
        if (strncmp(library, "stdlib", length) == 0)
        {
            registerIOSymbols(checker, parser, vm);
            registerMathSymbols(checker, parser, vm);
            registerTimeSymbols(checker, parser, vm);
            registerFileIOSymbols(checker, parser, vm);
            registerTypeSymbols(checker, parser, vm);
            registerUtilsSymbols(checker, parser, vm);
        }
        else if (strncmp(library, "io", length) == 0) { registerIOSymbols(checker, parser, vm); }
        else if (strncmp(library, "math", length) == 0) { registerMathSymbols(checker, parser, vm); }
        else if (strncmp(library, "chronos", length) == 0) { registerTimeSymbols(checker, parser, vm); }
        else if (strncmp(library, "fileIO", length)== 0)  { registerFileIOSymbols(checker, parser, vm); }
        else if (strncmp(library, "types", length)== 0){ registerTypeSymbols(checker, parser, vm); }
        else if (strncmp(library, "Hashmap", length) == 0){ registerHashMapSymbols(checker, parser, vm); }
        else if (strncmp(library, "ArrayList", length) == 0){ registerArrayListSymbols(checker, parser, vm); }
        else if (strncmp(library, "Strings", length) == 0){ registerStringSymbols(checker, parser, vm); }
        else if (strncmp(library, "utils", length) == 0){ registerUtilsSymbols(checker, parser, vm); }
        return;
    }
    else if (match(T_CLASS, parser))
    {
        consume(T_IDENTIFIER, "Please name your classes, they get lonely if you dont.", "SYNTAX ERROR", parser);
        const char* name = parser->previous.lexemeStart;
        int nameLength = parser->previous.length;

        //add in symbol and then get it for editing fields
        addSymbol(checker, name, nameLength, 0, VALUE_CLASS, NULL, parser);
        Symbol* classSymbol = lookUpSymbol(checker, name, nameLength);

        if (check(T_LEFT_BRACE, parser))
        {
            consume(T_LEFT_BRACE, "", "", parser); //consume the brace if it was found
            while (!check(T_RIGHT_BRACE, parser) && !check(T_EOF, parser))
            {
                if (match(T_MAKE, parser))
                {
                    //get the stuff fields and methods have in common
                    ValueType fieldType = getVarDeclarationType(parser, checker);
                    consume(T_IDENTIFIER, "Please name your inner class methods and variables, they get sad if you dont ;(.", "SYNTAX ERROR", parser);
                    const char* fieldName = parser->previous.lexemeStart;
                    int fieldNameLength = parser->previous.length;

                    if (check(T_LEFT_PAREN, parser))
                    {
                        advance(parser); //consume the '(' after seeing its there

                        //do methods
                        if (classSymbol->methodCount >= classSymbol->methodCapcaity)
                        {
                            //grow the methods array if needed
                            int newCapacity = classSymbol->methodCapcaity == 0 ? 4 : classSymbol->methodCapcaity * 2;
                            classSymbol->methodInfo = realloc(classSymbol->methodInfo, newCapacity * sizeof(CheckerMethodInfo));
                            classSymbol->methodCapcaity = newCapacity;
                        }

                        int slot = classSymbol->methodCount++;
                        classSymbol->methodInfo[slot].name = fieldName;
                        classSymbol->methodInfo[slot].length = fieldNameLength;
                        classSymbol->methodInfo[slot].name = fieldName;
                        classSymbol->methodInfo[slot].returnType = fieldType;
                        classSymbol->methodInfo[slot].paramCount = 0;

                        //parse the actually params
                        while (!check(T_RIGHT_PAREN, parser) && !check(T_EOF, parser))
                        {
                            ValueType paramType = getVarDeclarationType(parser, checker);
                            consume(T_IDENTIFIER, "Expected parameter name.", "SYNTAX ERROR", parser);

                            if (classSymbol->methodInfo[slot].paramCount < MAX_PARAMS)
                            {
                                ParamInfo* param = &classSymbol->methodInfo[slot].param[classSymbol->methodInfo[slot].paramCount++];
                                param->type = paramType;
                                param->name = parser->previous.lexemeStart;
                                param->length = parser->previous.length;
                            }

                            if (check(T_RIGHT_PAREN, parser)) break;
                            consume(T_COMMA, "Expected ',' between parameters inside of a class method declaration", "SYNTAX ERROR", parser);
                        }
                        consume(T_RIGHT_PAREN, "Expected a ')' to match every '(' in class method declarations", "SYNTAX ERROR", parser);

                        //now you can skip the body because its gonna get parser later
                        if (check(T_LEFT_BRACE, parser))
                        {
                            int braceCount = 1;
                            consume(T_LEFT_BRACE, "", "", parser);
                            while (braceCount > 0 && !check(T_EOF, parser))
                            {
                                if (match(T_LEFT_BRACE, parser)) braceCount++;
                                else if (match(T_RIGHT_BRACE, parser)) braceCount--;
                                else advance(parser);
                            }
                        }
                    }
                    else
                    {
                        //do fields
                        if (classSymbol->fieldCount >= classSymbol->fieldCapacity)
                        {
                            int newCap = classSymbol->fieldCapacity == 0 ? 4 : classSymbol->fieldCapacity * 2;
                            classSymbol->fieldsInfo = realloc(classSymbol->fieldsInfo,
                                                              newCap * sizeof(CheckerFieldInfo));
                            classSymbol->fieldCapacity = newCap;
                        }

                        //store it all directly in the class symbol array for that symbol
                        int slot = classSymbol->fieldCount++;
                        classSymbol->fieldsInfo[slot].type = fieldType;
                        classSymbol->fieldsInfo[slot].name = fieldName;
                        classSymbol->fieldsInfo[slot].length = fieldNameLength;
                        classSymbol->fieldsInfo[slot].className = NULL;
                        classSymbol->fieldsInfo[slot].classNameLength = 0;

                        //if the field is an instance then store it's class name it belongs to for type checking later
                        if (fieldType == VALUE_INSTANCE)
                        {
                            classSymbol->fieldsInfo[slot].className = checker->lastClassName;
                            classSymbol->fieldsInfo[slot].classNameLength = checker->lastClassNameLength;
                        }

                        //skip the other stuff
                        while (!check(T_SEMICOLON, parser) && !check(T_EOF, parser)) advance(parser);
                        consume(T_SEMICOLON, "", "", parser);
                    }
                }
                else
                {
                    advance(parser);
                }
            }
            consume(T_RIGHT_BRACE, "", "", parser);
        }
    }

    //skip to next function declaration
    while (!check(T_EOF, parser) && !check(T_MAKE, parser) && !check(T_HASH_PULLF, parser) && !check(T_CLASS, parser))
    {
        advance(parser);
    }
}

//------------------------------------------Compile function-----------------------------------------------------//
ObjFunction* compile(const char* source, Vm* vm)
{
    //set up the parser and essentially the scanner
    ASTparser parser;
    initParser(&parser, source);

    //second parser for the first pass through looking for declarations of classes and functions
    ASTparser parser2;
    initParser(&parser2, source);

    TypeChecker checker;
    initTypeChecker(&checker);

    //----------------------------First pass--------------------//
    while (!check(T_EOF, &parser2))
    {
        declareFunction(&parser2, &checker, vm );
    }
    if (parser2.hadError) return NULL;

#ifdef DEBUG_TRACE_EXECUTION
    printf("RAN PAST FIRST COMPILATION\n");
#endif

    //----------------------------Second pass--------------------//
    AstCompiler compiler;
    initFunctionCompiler(&compiler, NULL, "<script>", 8, VALUE_EMPTY, true, false, vm, &checker, &parser);

    while (!check(T_EOF, &parser))
    {
        declaration(&parser, &checker, &compiler, vm );
    }


    ObjFunction* topScript = endFunctionCompiler(&compiler, &parser, vm);
    pop(vm); //pop the top level script off the stack since gc no longer needs it

    //free all of the temp functions make in type checker
    freeTypeChecker(&checker);

    //emitReturn(&vm->chunk, &parser);
    return parser.hadError ? NULL : topScript;
}
