//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_ASTCOMPILER_H
#define OLI_NAT_ASTCOMPILER_H
#define UINT16_COUNT (UINT16_MAX + 1)
#define MAX_UPVALUES 256
#define MAX_LOCALS 256
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
    bool isCaptured;
} Local;

typedef struct
{
    int index;
    bool isLocal;
} UpValueInfo;

typedef struct AstCompiler
{
    Local locals[MAX_LOCALS];
    UpValueInfo upvalues[MAX_UPVALUES];
    int localCount;
    int scopeDepth;
    bool isTopLevel;
    ObjFunction* function; //that you write to
    struct AstCompiler* enclosing;
} AstCompiler;



ObjFunction* compile(const char* source, struct Vm* vm);
void initAstCompiler(AstCompiler* compiler);

int addUpValue(AstCompiler* compiler, int index, bool isLocal);
int resolveUpvalue(AstCompiler* compiler, const char* name, int length);

//shit forwarded to the bytecode compiler
void error(const char* message, const char* messageType, ASTparser* parser);
void errorAtCurrent(const char* message, const char* messageType, ASTparser* parser);
void errorAt(Token* token, const char* message, const char* messageType, ASTparser* parser);
void consume(TokenType type, const char* message, const char* messageType, ASTparser* parser);
void patchJump(int offset, Chunk* currentChunk, ASTparser* parser);
void emitReturn(Chunk* chunk, ASTparser* parser, struct Vm* vm);

#endif //OLI_NAT_ASTCOMPILER_H
