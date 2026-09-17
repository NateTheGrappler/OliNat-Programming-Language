//
// Created by natang on 5/29/26.
//

#ifndef OLI_NAT_DEBUG_H
#define OLI_NAT_DEBUG_H

#include "scanner.h"
#include "Expr.h"
#include "chunk.h"
void printToken(Token token, Scanner* scanner);
void printExpression(Expr* expr);
void printValue(Value value);

void disassembleChunk(Chunk* chunk, const char*  name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif //OLI_NAT_DEBUG_H
