//
// Created by natang on 5/29/26.
//

//add in a print function for tokens
#include "debug.h"
#include "common.h"
void printToken(Token token, Scanner* scanner)
{
    //Print out token / scanner information
    printf("Token lexeme '%.*s', Token type: '%d', Token Line: '%d', Token Length '%d'\n ",
    token.length, token.lexemeStart, token.type, token.line, token.length);
}


//a recursive function for printing out expressions using polish (hehe) notation
void printExpression(Expr* expr)
{
    if (expr == NULL)
    {
        printf("Expression passed in was null");
    }

    switch (expr->type)
    {
        case EXPR_LITERAL:
        {

            //small switch cases for prettier printing out of types
            const char* type;
            switch (expr->literal.type)
            {
                case VALUE_INT:    type= "INT"; break;
                case VALUE_DOUBLE: type= "DOUBLE"; break;
                case VALUE_FLOAT:  type= "FLOAT"; break;
            }

            switch (expr->literal.type)
            {
                //print out both the actual value as well as the associated type (for type checking later on)
                case VALUE_INT:
                {
                    printf("%s-%d", type,  expr->literal.value.integer_val);
                    break;
                }
                case VALUE_FLOAT:
                {
                    printf("%s-%f", type, expr->literal.value.float_val);
                    break;
                }
                case VALUE_DOUBLE:
                {
                    printf("%s-%lf", type, expr->literal.value.double_val);
                    break;
                }
            }
            break;
        }
        case EXPR_UNARY:
        {
            printf("(%c", expr->unary.operator);
            printExpression(expr->unary.right);
            printf(")");
            break;
        }
        case EXPR_GROUPING:
        {
            printf("(group ");
            printExpression(expr->grouping.expr);
            printf(")");
            break;
        }
        case EXPR_BINARY:
        {
            printf("(%c ",  expr->binary.operator);
            printExpression(expr->binary.left);
            printf(" ");
            printExpression(expr->binary.right);
            printf(")");
            break;
        }
    }
}
