//
// Created by natang on 5/29/26.
//

//add in a print function for tokens
#include "debug.h"
#include "common.h"
void printToken(Token token, Scanner* scanner)
{
    //Print out token / scanner information
    printf("Token lexeme '%s', Token type: '%d', Token Line: '%d', Token Length '%d'\n ",  token.lexemeStart, token.type, token.line, token.length);

}
