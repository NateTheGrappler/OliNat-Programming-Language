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

typedef struct
{
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} AstCompiler;



bool compile(const char* source, struct Vm* vm);
void initAstCompiler(AstCompiler* compiler);

//shit forwarded to the bytecode compiler
void error(const char* message, ASTparser* parser);
void errorAtCurrent(const char* message, ASTparser* parser);
void errorAt(Token* token, const char* message, ASTparser* parser);
void consume(TokenType type, const char* message, ASTparser* parser);
void emitReturn(Chunk* chunk, ASTparser* parser);

#endif //OLI_NAT_ASTCOMPILER_H
