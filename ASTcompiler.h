//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_ASTCOMPILER_H
#define OLI_NAT_ASTCOMPILER_H
#define UINT8_COUNT (UINT8_MAX + 1)
#include "scanner.h"
#include "common.h"
#include "Expr.h"
#include "debug.h"
#include "typeChecker.h"
#include "object.h"

struct Vm;

//Parser struct for making ASTs
typedef struct ASTparser
{
    Scanner scanner;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} ASTparser;

typedef struct {
    const char* name;
    int length;
    int depth;
} Local;

typedef struct AstCompiler
{
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
    bool isTopLevel;
    ObjFunction* function; //that you write to
    struct AstCompiler* enclosing;
} AstCompiler;



ObjFunction* compile(const char* source, struct Vm* vm);
void initAstCompiler(AstCompiler* compiler);

//shit forwarded to the bytecode compiler
void error(const char* message, const char* messageType, ASTparser* parser);
void errorAtCurrent(const char* message, const char* messageType, ASTparser* parser);
void errorAt(Token* token, const char* message, const char* messageType, ASTparser* parser);
void consume(TokenType type, const char* message, const char* messageType, ASTparser* parser);
void emitReturn(Chunk* chunk, ASTparser* parser);

#endif //OLI_NAT_ASTCOMPILER_H
