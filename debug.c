//
// Created by natang on 5/29/26.
//

//add in a print function for tokens
#include "debug.h"
#include "common.h"
#include "Expr.h"
#include "object.h"

void printToken(Token token, Scanner* scanner)
{
    //Print out token / scanner information
    printf("Token lexeme '%.*s', Token type: '%d', Token Line: '%d', Token Length '%d'\n ",
    token.length, token.lexemeStart, token.type, token.line, token.length);
}
void printExpression(Expr* expr) //a recursive function for printing out expressions using polish (hehe) notation
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
                case VALUE_INT:     type= "INT"; break;
                case VALUE_DOUBLE:  type= "DOUBLE"; break;
                case VALUE_FLOAT:   type= "FLOAT"; break;
                case VALUE_BOOL:    type= "BOOL"; break;
                case VALUE_STRING:  type= "STR"; break;
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
                case VALUE_STRING:
                {
                    printf("%s-%s", type, expr->literal.value.string_val);
                    break;
                }
                case VALUE_BOOL:
                {
                    printf("%s-%s", type, expr->literal.value.boolean_val ? "true" : "false");
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
            printf("(%s ",  expr->binary.operator);
            printExpression(expr->binary.left);
            printf(" ");
            printExpression(expr->binary.right);
            printf(")");
            break;
        }
        case EXPR_VARIABLE:
        {
            printf("IDENTIFIER-%.*s", expr->variable.length, expr->variable.name);
            break;
        }
    }
    //printf("\n");
}

void printObject(Value value)
{
    switch (value.as.object_val->type)
    {
        case OBJ_STRING:
        {
            printf("%s", AS_CSTRING(value));
            break;
        }
    }
}

void printValue(Value value)
{
    switch (value.type)
    {
        case VALUE_BOOL:   printf(GET_BOOL_VAL(value) ? "true" : "false"); break;
        case VALUE_DOUBLE: printf("%d", GET_DOUBLE_VAL(value)); break;
        case VALUE_INT:    printf("%d", GET_INT_VAL(value)); break;
        case VALUE_FLOAT:  printf("%f", GET_FLOAT_VAL(value)); break;
        case VALUE_OBJECT: printObject(value); break;
        case VALUE_EMPTY: break;
    }
}

//-----------------CHUNK VISUALIZATION----------------//

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->byteCode[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2; //since the constants hold both an index and a value, you go over two spots in the memory
}
static int simpleInstruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}
static int byteInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t slot = chunk->byteCode[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

//actually main printing function
int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset); //prints the byte offset of the given instruction

    //print the line of sourcecode that the bytecode came from
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset-1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->byteCode[offset];
    switch (instruction)
    {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_NOT_EQUAL:
            return simpleInstruction("OP_NOT_EQUAL", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_GREATER_EQUAL:
            return simpleInstruction("OP_GREATER_EQUAL", offset);
        case OP_LESS_EQUAL:
            return simpleInstruction("OP_LESS_EQUAL", offset);
    }
}