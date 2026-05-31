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
            switch (expr->literal.type)
            {
                case VALUE_INT:
                {
                    printf("%d", expr->literal.value.integer_val);
                    break;
                }
                case VALUE_FLOAT:
                {
                    printf("%f", expr->literal.value.float_val);
                    break;
                }
                case VALUE_DOUBLE:
                {
                    printf("%lf", expr->literal.value.double_val);
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
