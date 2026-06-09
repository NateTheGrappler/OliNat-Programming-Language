//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_BYTECOMPILER_H
#define OLI_NAT_BYTECOMPILER_H

#include "Expr.h"
#include "ASTcompiler.h"
#include "vm.h"
#include "chunk.h"

void compileExpressionByte(Expr* expr, ASTparser* parser, Chunk* vmChunk, AstCompiler* compiler, Vm* vm);
void compileBytecode(Expr* expr, ASTparser* parser, Chunk* vmChunk, AstCompiler* compiler, Vm* vm);
void emitDefineGlobal(const char* name, int length, Chunk* chunk, ASTparser* parser, Vm* vm);
void emitByte(uint8_t byte, Chunk* chunk, ASTparser* parser);
short emitJump(uint8_t instruction, Chunk* chunk, ASTparser* parser);
void emitConstant(Value value, Chunk* chunk, ASTparser* parser, Vm* vm);
void emitReturnToChunk(Chunk* chunk, ASTparser* parser);

#endif //OLI_NAT_BYTECOMPILER_H
