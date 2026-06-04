//
// Created by natang on 5/30/26.
//

#include "ASTcompiler.h"

#include "Expr.h"
#include "Bytecompiler.h"

//-------DECLARATIONS------------//
static void advance(ASTparser* parser);
static Expr* astExpression(ASTparser* parser);
static Expr* number(bool canAssign, ASTparser* parser);
static Expr* boolean(bool canAssign, ASTparser* parser);
static Expr* unary(bool canAssign, ASTparser* parser);
static Expr*  grouping(bool canAssign, ASTparser* parser);
static Expr* binary(bool canAssign, ASTparser* parser, Expr* left);

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
typedef Expr* (*PrefixFn)(bool canAssign, ASTparser* parser);
typedef Expr* (*InfixFn)(bool canAssign, ASTparser* parser, Expr* left);

typedef struct
{
    PrefixFn prefix;
    InfixFn infix;
    Precedence precedence;
} ParseRule;

ParseRule rules[] = {
  [T_LEFT_PAREN]    = {grouping,     NULL,   PREC_CALL},
  [T_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [T_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
  [T_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [T_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [T_DOT]           = {NULL,     NULL,    PREC_CALL},
  [T_MINUS]         = {unary,     binary, PREC_TERM},
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
  [T_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},

    //TODO: check types
  [T_STRING]        = {NULL,     NULL,   PREC_NONE},
  [T_FLOAT]         = {NULL,     NULL,   PREC_NONE},
  [T_DOUBLE]        = {NULL,     NULL,   PREC_NONE},
  [T_INTEGER]       = {NULL,     NULL,   PREC_NONE},
  [T_BOOL]          = {NULL,     NULL,   PREC_NONE},
  [T_STRING_VAL]    = {NULL,     NULL,   PREC_NONE},
  [T_FLOAT_VAL]     = {number,   NULL,   PREC_NONE},
  [T_DOUBLE_VAL]    = {number,   NULL,   PREC_NONE},
  [T_INTEGER_VAL]   = {number,   NULL,   PREC_NONE},

  [T_AND]           = {NULL,     NULL,   PREC_AND},
  [T_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [T_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [T_FALSE]         = {boolean,  NULL,   PREC_NONE},
  [T_FOR]           = {NULL,     NULL,   PREC_NONE},
  [T_FUN]           = {NULL,     NULL,   PREC_NONE},
  [T_IF]            = {NULL,     NULL,   PREC_NONE},
  [T_EMPTY]         = {NULL,     NULL,   PREC_NONE},
  [T_OR]            = {NULL,     NULL,   PREC_OR},
  [T_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [T_PULLF]         = {NULL,     NULL,   PREC_NONE},
  [T_THIS]          = {NULL,     NULL,   PREC_NONE},
  [T_TRUE]          = {boolean,  NULL,   PREC_NONE},
  [T_MAKE]          = {NULL,     NULL,   PREC_NONE},
  [T_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [T_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [T_EOF]           = {NULL,     NULL,   PREC_NONE},
};


//-------DECLARATIONS------------//
static Expr* parserPrecedence(Precedence precedence, ASTparser* parser);
static ParseRule* getRule(TokenType type);

//------------------------------------------Init functions-----------------------------------------------------//
void initAstCompiler(AstCompiler* comlpiler)
{

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
void errorAt(Token* token, const char* message, ASTparser* parser)
{
    if (parser->panicMode) return; //skip if error already detected

    fprintf(stderr, "[line %d Error", token->line);

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

    fprintf(stderr, ": %s\n]", message);
    parser->hadError = true;
    parser->panicMode = true;
}
//error at previous token
void error(const char* message, ASTparser* parser)
{
    errorAt(&parser->previous, message, parser);
}
//error at current looking at token
void errorAtCurrent(const char* message, ASTparser* parser)
{
    errorAt(&parser->current, message, parser);
}
//my personal favorite function, check for necessary token
void consume(TokenType type, const char* message, ASTparser* parser)
{
    if (parser->current.type == type)
    {
        advance(parser);
        return;
    }
    errorAtCurrent(message, parser);
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
        if (parser->current.type != T_ERROR) break;
        errorAtCurrent(parser->current.lexemeStart, parser);
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

static Expr* boolean(bool canAssign, ASTparser* parser)
{
    switch (parser->previous.type)
    {
        case T_TRUE: createLiteralBool(true, parser->previous.line); break;
        case T_FALSE: createLiteralBool(false, parser->previous.line); break;
        default: return NULL; //unreachable (hopefully)
    }
}
//literal expression parsing
static Expr* number(bool canAssign, ASTparser* parser)
{
    //handle all three possible numbers you can have
    switch (parser->previous.type)
    {
        case T_DOUBLE_VAL:
        {
            double value = strtod(parser->previous.lexemeStart, NULL);
            return createLiteralDouble(value, parser->previous.line);
        }
        case T_FLOAT_VAL:
        {
            float value = strtof(parser->previous.lexemeStart, NULL);
            return createLiteralFloat(value, parser->previous.line);
        }
        case T_INTEGER_VAL:
        {
            long value = strtol(parser->previous.lexemeStart, NULL, 10);
            return createLiteralInt((int)value, parser->previous.line);
        }
        default:
            return NULL; //unreachable (hopefully)

    }
}
static Expr* unary(bool canAssign, ASTparser* parser)
{
    //get unary type
    TokenType operatorType = parser->previous.type;

    //recursive call to consume next literal
    Expr* right = parserPrecedence(PREC_UNARY, parser);

    switch (operatorType)
    {
        case T_MINUS:
        {
            return createUnary('-', right, parser->previous.line);
        }
        case T_BANG:
        {
            //TODO: implement whenever you have strings and bools
            return createUnary('!', right, parser->previous.line);
        }
        default: return NULL; //unreachable
    }
}
static Expr*  grouping(bool canAssign, ASTparser* parser)
{
    Expr* expr = astExpression(parser);
    consume(T_RIGHT_PAREN, "Please finish all parentheses with a ')'.", parser);
    return expr;
}
static Expr* binary(bool canAssign, ASTparser* parser, Expr* left)
{
    //compile the second literal and also check out the type of operation you got
    TokenType operatorType = parser->previous.type;
    ParseRule* rule = getRule(operatorType);
    Expr* right = parserPrecedence((Precedence)(rule->precedence+1), parser);

    switch (operatorType)
    {
        case T_PLUS:          return createBinary(left, right, "+", parser->previous.line);
        case T_MINUS:         return createBinary(left, right, "-", parser->previous.line);
        case T_STAR:          return createBinary(left, right, "*", parser->previous.line);
        case T_SLASH:         return createBinary(left, right, "/", parser->previous.line);
        case T_BANG_EQUAL:    return createBinary(left, right, "!=", parser->previous.line);
        case T_EQUAL_EQUAL:   return createBinary(left, right, "==", parser->previous.line);
        case T_GREATER:       return createBinary(left, right, ">", parser->previous.line);
        case T_GREATER_EQUAL: return createBinary(left, right, ">=", parser->previous.line);
        case T_LESS:          return createBinary(left, right, "<", parser->previous.line);
        case T_LESS_EQUAL:    return createBinary(left, right, "<=", parser->previous.line);
    }
}

//actual parser precedence stuff
static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}
static Expr* parserPrecedence(Precedence precedence, ASTparser* parser)
{
    //advance to next token, get the rule for the literal value and then some
    //error catching for bad snytax
    advance(parser);
    PrefixFn prefixRule = getRule(parser->previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expected expression", parser);
        return NULL;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT; //for when i add variables
    Expr* left = prefixRule(precedence <= PREC_ASSIGNMENT, parser);

    //consume the actual expression operator and what not, lead the recursive portion of
    //this setup
    while (precedence <= getRule(parser->current.type)->precedence)
    {
        advance(parser);
        InfixFn infixRule = getRule(parser->previous.type)->infix;
        left = infixRule(canAssign, parser, left);
    }

    return left;
}
static Expr* astExpression(ASTparser* parser)
{
    return parserPrecedence(PREC_ASSIGNMENT, parser);
}


//------------------------------------------Compile function-----------------------------------------------------//
bool compile(const char* source, Vm* vm)
{
    //set up the parser and essentially the scanner
    ASTparser parser;
    initParser(&parser, source);

    TypeChecker checker;
    initTypeChecker(&checker);

    //print out each of the ASTs
    Expr* expr = astExpression(&parser);
    ValueType type = checkExpression(&checker, expr);
    if (type == VALUE_ERROR)
    {
        return true; //TODO: add in some type checking error messages or something
    }
    compileBytecode(expr, &parser, &vm->chunk, vm);

    //set up compiler
    //AstCompiler compiler;
    //initAstCompiler(&compiler);

    return parser.hadError;
}
