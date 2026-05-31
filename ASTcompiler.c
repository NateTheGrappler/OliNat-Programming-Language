//
// Created by natang on 5/30/26.
//

#include "ASTcompiler.h"
#include "scanner.h"
#include "common.h"



//------------------------------------------Structs holding all fun data-----------------------------------------------------//
//Parser struct for making ASTs
typedef struct
{
    Scanner scanner;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} ASTparser;

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

typedef struct
{
    //for local vars?
} AstCompiler;

//-------DECLARATIONS------------//
static void advance(ASTparser* parser);


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
static void errorAt(Token* token, const char* message, ASTparser* parser)
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

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
    parser->panicMode = true;
}
//error at previous token
static void error(const char* message, ASTparser* parser)
{
    errorAt(&parser->previous, message, parser);
}
//error at current looking at token
static void errorAtCurrent(const char* message, ASTparser* parser)
{
    errorAt(&parser->current, message, parser);
}
//my personal favorite function, check for necessary token
static void consume(TokenType type, const char* message, ASTparser* parser)
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





//------------------------------------------Compile function-----------------------------------------------------//
bool compile(const char* source)
{
    //set up the parser and essentially the scanner
    ASTparser parser;
    initParser(&parser, source);

    //TODO: implement code that uses prat parsing to compile expressions

    //set up compiler
    AstCompiler compiler;
    initAstCompiler(&compiler);

    //TODO: implement code that uses prat parsing to compile expressions

    return parser.hadError;
}
