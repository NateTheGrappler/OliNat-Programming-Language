//
// Created by natang on 5/30/26.
//
#include "Bytecompiler.h"
#include "common.h"
#include "object.h"

//--------Declarations------------//
static uint8_t makeConstant(Value value, Chunk* chunk, Vm* vm, ASTparser* parser);

//-------------------------------------Writting to the chunk functions------------------------------------------//
static void emitByte(uint8_t byte, Chunk* chunk, ASTparser* parser)
{
    writeToChunk(chunk, byte, parser->previous.line);
}
static void emitBytes(uint8_t byteOne, uint8_t byteTwo, Chunk* chunk, ASTparser* parser)
{
    emitByte(byteOne, chunk, parser);
    emitByte(byteTwo, chunk, parser);
}
static void emitReturn(Chunk* chunk, ASTparser* parser)
{
    emitByte(OP_RETURN, chunk, parser);
}
static void emitConstant(Value value, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    emitBytes(OP_CONSTANT, makeConstant(value, chunk, vm, parser), chunk, parser);
}

static void emitBinaryOperator(const char* operator, Chunk* chunk, ASTparser* parser)
{
    //TODO: see if theres a better way than a bunch of if statements
    //math ops
    if (strcmp(operator, "+") == 0) {emitByte(OP_ADD, chunk, parser);      return;}
    if (strcmp(operator, "-") == 0) {emitByte(OP_SUBTRACT, chunk, parser); return;}
    if (strcmp(operator, "/") == 0) {emitByte(OP_DIVIDE, chunk, parser);   return;}
    if (strcmp(operator, "*") == 0) {emitByte(OP_MULTIPLY, chunk, parser); return;}

    //comparisions
    if (strcmp(operator, ">") == 0)  {emitByte(OP_GREATER, chunk, parser);       return;}
    if (strcmp(operator, "<") == 0)  {emitByte(OP_LESS, chunk, parser);          return;}
    if (strcmp(operator, ">=") == 0) {emitByte(OP_GREATER_EQUAL, chunk, parser); return;}
    if (strcmp(operator, "<=") == 0) {emitByte(OP_LESS_EQUAL, chunk, parser);    return;}
    if (strcmp(operator, "==") == 0) {emitByte(OP_EQUAL, chunk, parser);         return;}
    if (strcmp(operator, "!=") == 0) {emitByte(OP_NOT_EQUAL, chunk, parser);     return;}

    error("Invalid operation found in one of your math operations", parser);
}
static void emitUnaryOperator(char operator, Chunk* chunk, ASTparser* parser)
{
    switch (operator)
    {
        case '-': emitByte(OP_NEGATE,  chunk, parser); break;
        case '!': emitByte(OP_INVERSE, chunk, parser); break;
        default:
        {
            error("Invalid unary operator found in expressions, please double check what you are doing.", parser);
            return;
        }
    }
    //hopefully unreachable
}


//-----------------------------------------__HELPER FUNCITONS-----------------------------------------------///

static uint8_t makeConstant(Value value, Chunk* chunk, Vm* vm, ASTparser* parser)
{
    int constantIndex = addConstant(chunk, value, vm);
    if (constantIndex > UINT8_MAX)
    {
        error("Too many constants in one chunk.", parser); //TODO: add in a way to store more constants than 255 in one chunk
    }
    return (uint8_t)constantIndex;
}


//----------------------------------------Actual compiler stuff-------------------------------------------------//

void compileExpressionByte(Expr* expr, ASTparser* parser, Chunk* vmChunk, Vm* vm)
{

    #ifdef DEBUG_TRACE_EXECUTION
        printf("Ran inside of compile expression byte for expression: ");
        printExpression(expr);
        printf("\n");
    #endif

    if (expr == NULL)
    {
        error("NULL expression reached in compiler", parser);
        return;
    }

    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            compileExpressionByte(expr->binary.left, parser, vmChunk, vm);
            compileExpressionByte(expr->binary.right, parser, vmChunk, vm);
            emitBinaryOperator(expr->binary.operator, vmChunk, parser);
            break;
        }
        case EXPR_LITERAL:
        {
            switch (expr->literal.type)
            {
                case VALUE_BOOL:   emitConstant(CREATE_BOOL_VAL(expr->literal.value.boolean_val), vmChunk, parser, vm);  break;
                case VALUE_INT:    emitConstant(CREATE_INT_VAL(expr->literal.value.integer_val), vmChunk, parser, vm);   break;
                case VALUE_FLOAT:  emitConstant(CREATE_FLOAT_VAL(expr->literal.value.float_val), vmChunk, parser, vm);   break;
                case VALUE_DOUBLE: emitConstant(CREATE_DOUBLE_VAL(expr->literal.value.double_val), vmChunk, parser, vm); break;
                case VALUE_STRING:
                {
                    //copy the copied string in the expression object, then basically get the length of it (a bit inefficent but whatever)
                    //and then toss the vm in there, basically getting that raw c char* to an ObjString struct to a Value struct
                    emitConstant(CREATE_OBJECT_VAL((Obj*)copyString(expr->literal.value.string_val,
              strlen(expr->literal.value.string_val), vm)), vmChunk, parser, vm);
                    break;
                }
            }
            break;
        }
        case EXPR_UNARY:
        {
            compileExpressionByte(expr->unary.right, parser, vmChunk, vm);
            emitUnaryOperator(expr->unary.operator, vmChunk, parser);
            break;
        }
        case EXPR_GROUPING:
        {
            //just pass it on
            compileExpressionByte(expr->grouping.expr, parser, vmChunk, vm);
            break;
        }
    }
}


void compileBytecode(Expr* expr, ASTparser* parser, Chunk* vmChunk, Vm* vm)
{
    //TODO: add in a way to seperate consequtive expressions from return call
    compileExpressionByte(expr, parser, vmChunk, vm);
    emitReturn(vmChunk, parser);
}
