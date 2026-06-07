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
static Expr* string(bool canAssign, ASTparser* parser);
static Expr* unary(bool canAssign, ASTparser* parser);
static Expr*  grouping(bool canAssign, ASTparser* parser);
static Expr* binary(bool canAssign, ASTparser* parser, Expr* left);
static Expr* variable(bool canAssign, ASTparser* parser);
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
void initAstCompiler(AstCompiler* compiler)
{
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
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
        //printToken(parser->current, &parser->scanner);
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


//literal expression parsing
static Expr* variable(bool canAssign, ASTparser* parser)
{
    //save the name of the var
    const char* name = parser->previous.lexemeStart;
    int length = parser->previous.length;
    int line = parser->previous.line;

    if (match(T_EQUAL, parser) && canAssign)
    {
        Expr* value = astExpression(parser);
        return createVarAssignment(name, length, value, line);
    }
    return createVariable(name, length, line);
}
static Expr* string(bool canAssign, ASTparser* parser)
{
    //copy the string because the source code representation might still be needed
    int length = parser->previous.length - 2;
    char* chars = ALLOCATE(char, length+1);
    memcpy(chars, parser->previous.lexemeStart + 1, length);
    chars[length] = '\0';
    return createLiteralString(chars, parser->previous.line);
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
static Expr* grouping(bool canAssign, ASTparser* parser)
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


//------Variable Items-------//
static ValueType getVarDeclarationType(ASTparser* parser)
{
    switch (parser->current.type)
    {
        case T_INTEGER:
        {
            consume(T_INTEGER, "Please declare the type of the variable you wish to create after 'make'.", parser);
            return VALUE_INT;
        }
        case T_FLOAT:
        {
            consume(T_FLOAT, "Please declare the type of the variable you wish to create after 'make'.", parser);
            return VALUE_FLOAT;
        }
        case T_DOUBLE:
        {
            consume(T_DOUBLE, "Please declare the type of the variable you wish to create after 'make'.", parser);
            return VALUE_DOUBLE;
        }
        case T_STRING:
        {
            consume(T_STRING, "Please declare the type of the variable you wish to create after 'make'.", parser);
            return VALUE_STRING;
        }
        case T_BOOL:
        {
            consume(T_BOOL, "Please declare the type of the variable you wish to create after 'make'.", parser);
            return VALUE_BOOL;
        }
        default:
            error("Expected a type after 'make'", parser);
            return VALUE_ERROR;
    }
}
static void varDeclaration(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{

    //get / consume the declaraed type
    ValueType type = getVarDeclarationType(parser);

    //get the var name
    consume(T_IDENTIFIER, "You must name your variable, they get sad if you dont.", parser);
    const char* name = parser->previous.lexemeStart;
    int nameLength = parser->previous.length;

    consume(T_EQUAL, "Expected a '=' after you declare a new variable.", parser);
    Expr* varInitializer = astExpression(parser);
    consume(T_SEMICOLON, "Expected ';' after you declare a new variable", parser);

    //check the type of the expression vs the declared type
    ValueType realType = checkExpression(checker, varInitializer);
    if (realType != type)
    {
        error("A variable expression's type must be the same as it's declared type.", parser);
    }


    if (compiler->scopeDepth == 0)
    {
        //set the global to get defined here
        addSymbol(checker, name, nameLength, compiler->scopeDepth, type, parser);;
        compileBytecode(varInitializer, parser, &vm->chunk, compiler, vm);
        emitDefineGlobal(name, nameLength, &vm->chunk, parser, vm);
    }
    else
    {
        //declare a new local and then actually parse the value
        Local* local = &compiler->locals[compiler->localCount++];
        local->name = name;
        local->length = nameLength;
        local->depth = compiler->scopeDepth;

        //the compiled value ends up on the stack and IS the local
        addSymbol(checker, name, nameLength, compiler->scopeDepth, type, parser);
        compileBytecode(varInitializer, parser, &vm->chunk, compiler, vm);
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
        emitByte(OP_POP, &vm->chunk, parser);
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

    consume(T_RIGHT_BRACE, "Each of your '{' must inturn have a matching '}', please.", parser);
}


//if statements
static void patchJump(int offset, Chunk* currentChunk, ASTparser* parser)
{
    short jump = currentChunk->count - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over twin.", parser);
    }

    currentChunk->byteCode[offset] = (jump >> 8) & 0xff;
    currentChunk->byteCode[offset+1] = jump & 0xff;
}
static void ifStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    consume(T_LEFT_PAREN, "Please supplement your if statement with a '(' after 'if'.", parser);
    Expr* condition = astExpression(parser);
    consume(T_RIGHT_PAREN, "Please close your if statement's condition with ')'.", parser);


    ValueType conditionType = checkExpression(checker, condition);
    if (conditionType != VALUE_BOOL)
    {
        error("If condition must be a boolean.", parser);
    }
    compileBytecode(condition, parser, &vm->chunk, compiler, vm);


    //emit a jump instruction with then two supplementary bytes to store how large the jump is
    short jumpIndex = emitJump(OP_JUMP_IF_FALSE, &vm->chunk, parser);
    emitByte(OP_POP, &vm->chunk, parser);         //get condition value off stack TODO: see if this causes issues
    statement(parser, checker, compiler, vm);     //parse block

    short elseJump = emitJump(OP_JUMP, &vm->chunk, parser); //always jump if found

    patchJump(jumpIndex, &vm->chunk, parser);
    emitByte(OP_POP, &vm->chunk, parser);


    if (match(T_ELSE, parser))
    {
        statement(parser, checker, compiler, vm);     //parse else block
    }
    patchJump(elseJump, &vm->chunk, parser);

}


//while loops and for loops
static void emitLoop(int loopStart, ASTparser* parser, Vm* vm)
{
    emitByte(OP_LOOP, &vm->chunk, parser);

    int offset = vm->chunk.count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body is too large.", parser);

    emitByte(((offset >> 8) & 0xff), &vm->chunk, parser);
    emitByte((offset  & 0xff), &vm->chunk, parser);
}
static void whileStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    int loopStart = vm->chunk.count;

    consume(T_LEFT_PAREN, "Please supplement your while statement with a '(' after 'while'.", parser);
    Expr* condition = astExpression(parser);
    consume(T_RIGHT_PAREN, "Please close your while statement's condition with ')'.", parser);


    ValueType conditionType = checkExpression(checker, condition);
    if (conditionType != VALUE_BOOL)
    {
        error("While condition must be a boolean.", parser);
    }
    compileBytecode(condition, parser, &vm->chunk, compiler, vm);

    int exitJump = emitJump(OP_JUMP_IF_FALSE, &vm->chunk, parser);
    emitByte(OP_POP, &vm->chunk, parser);
    statement(parser, checker, compiler, vm);
    emitLoop(loopStart, parser, vm);

    patchJump(exitJump, &vm->chunk, parser);
    emitByte(OP_POP, &vm->chunk, parser);
}
static void forStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    beginScope(compiler);
    consume(T_LEFT_PAREN, "Please use a '(' character after the for keyword.", parser);

    if (match(T_SEMICOLON, parser))
    {
        //no
        //initializer, infinite loop
    }
    else if (match(T_MAKE, parser))
    {
        varDeclaration(parser, checker, compiler, vm);
    }
    else
    {
        expressionStatement(parser, checker, compiler, vm);
    }

    int loopStart = vm->chunk.count;

    int exitJump = -1;
    if (!match(T_SEMICOLON, parser))
    {
        //calculate the actual condition
        Expr* expr = astExpression(parser);
        ValueType type = checkExpression(checker, expr);
        if (type != VALUE_BOOL) { error("A for loops condition must evaluate to a boolean expression.", parser); }

        compileBytecode(expr, parser, &vm->chunk, compiler, vm);
        consume(T_SEMICOLON, "The code expects ';' after a for loop conditional.", parser);

        exitJump = emitJump(OP_JUMP_IF_FALSE, &vm->chunk, parser);
        emitByte(OP_POP, &vm->chunk, parser);
    }

    int bodyJump = -1;
    int incrementStart = -1;
    if (!match(T_RIGHT_PAREN, parser))
    {
        //evaluate the expression changing the loop
        bodyJump = emitJump(OP_JUMP, &vm->chunk, parser);
        incrementStart = vm->chunk.count;

        Expr* expr = astExpression(parser);
        compileBytecode(expr,parser, &vm->chunk, compiler, vm);

        emitByte(OP_POP, &vm->chunk, parser);
        consume(T_RIGHT_PAREN, "Expect ')' after for loop clauses", parser);

        emitLoop(loopStart, parser, vm);
        loopStart = incrementStart;
        patchJump(bodyJump, &vm->chunk, parser);
    }
    else
    {
        consume(T_RIGHT_PAREN, "Expect ')' after for clauses", parser);
    }

    statement(parser, checker, compiler, vm);
    emitLoop(loopStart, parser, vm);

    if (exitJump != -1)
    {
        patchJump(exitJump, &vm->chunk, parser);
        emitByte(OP_POP, &vm->chunk, parser);
    }

    endScope(compiler, checker, vm, parser);
}

//Basic statements
static void expressionStatement(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    //compile and check expression, then compile again to bytecode
    Expr* expr = astExpression(parser);
    ValueType type = checkExpression(checker, expr);

    if (type != VALUE_ERROR)
    {
        compileBytecode(expr, parser, &vm->chunk, compiler, vm);
        emitByte(OP_POP, &vm->chunk, parser);
    }

    //consume the ; and free memory
    consume(T_SEMICOLON, "Please end all of your expressions with a ';'!", parser);
    freeExpr(expr);
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
    else
    {
        expressionStatement(parser, checker, compiler, vm);
    }
}



//newer stuff for the statements and variables
static void declaration(ASTparser* parser, TypeChecker* checker, AstCompiler* compiler, Vm* vm)
{
    if (match(T_MAKE, parser))
    {
        varDeclaration(parser, checker, compiler, vm);
    }
    else
    {
        statement(parser, checker, compiler, vm);
    }
}


//------------------------------------------Compile function-----------------------------------------------------//
bool compile(const char* source, Vm* vm)
{
    //set up the parser and essentially the scanner
    ASTparser parser;
    initParser(&parser, source);

    TypeChecker checker;
    initTypeChecker(&checker);

    AstCompiler compiler;
    initAstCompiler(&compiler);

    while (!check(T_EOF, &parser))
    {
        declaration(&parser, &checker, &compiler, vm );
    }



    emitReturn(&vm->chunk, &parser);
    return parser.hadError;
}
