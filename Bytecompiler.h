//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_BYTECOMPILER_H
#define OLI_NAT_BYTECOMPILER_H

#include "Expr.h"
#include "ASTcompiler.h"
#include "vm.h"
#include "chunk.h"

void compileExpressionByte(Expr* expr, ASTparser* parser, Chunk* vmChunk, Vm* vm);
void compileBytecode(Expr* expr, ASTparser* parser, Chunk* vmChunk, Vm* vm);

#endif //OLI_NAT_BYTECOMPILER_H
