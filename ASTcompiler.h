//
// Created by natang on 5/30/26.
//

#ifndef OLI_NAT_ASTCOMPILER_H
#define OLI_NAT_ASTCOMPILER_H

#include "scanner.h"
#include "common.h"
#include "Expr.h"
#include "debug.h"

//Parser struct for making ASTs
typedef struct
{
    Scanner scanner;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} ASTparser;

typedef struct
{
    //for local vars?
} AstCompiler;



bool compile(const char* source);

#endif //OLI_NAT_ASTCOMPILER_H
