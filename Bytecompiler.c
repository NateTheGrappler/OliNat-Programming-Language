//
// Created by natang on 5/30/26.
//
#include "Bytecompiler.h"
#include "common.h"
#include "object.h"

//--------Declarations------------//
static uint8_t makeConstant(Value value, Chunk* chunk, Vm* vm, ASTparser* parser);

//-------------------------------------Writting to the chunk functions------------------------------------------//
void emitByte(uint8_t byte, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    writeToChunk(chunk, byte, parser->previous.line, vm);
}
static void emitBytes(uint8_t byteOne, uint8_t byteTwo, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    emitByte(byteOne, chunk, parser, vm);
    emitByte(byteTwo, chunk, parser, vm);
}
void emitReturn(Chunk* chunk, ASTparser* parser, Vm* vm)
{
    emitByte(OP_RETURN, chunk, parser, vm);
}
void emitConstant(Value value, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    int constantIndex = addConstant(chunk, value, vm);
    if (constantIndex <= UINT8_MAX)
    {
        emitBytes(OP_CONSTANT, (uint8_t)constantIndex, chunk, parser, vm);
    }
    else if (constantIndex <= UINT16_MAX)
    {
        emitByte(OP_CONSTANT_LONG, chunk, parser, vm);
        emitByte((constantIndex >> 8) & 0xff, chunk, parser, vm);
        emitByte(constantIndex & 0xff, chunk, parser, vm);
    }
    else
    {
        error("Too many constants in bytecode chunk", "MEMORY ERROR", parser);
    }
}

static void emitBinaryOperator(const char* operator, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    //math ops
    if (strcmp(operator, "+") == 0) {emitByte(OP_ADD, chunk, parser, vm);      return;}
    if (strcmp(operator, "-") == 0) {emitByte(OP_SUBTRACT, chunk, parser, vm); return;}
    if (strcmp(operator, "/") == 0) {emitByte(OP_DIVIDE, chunk, parser, vm);   return;}
    if (strcmp(operator, "*") == 0) {emitByte(OP_MULTIPLY, chunk, parser, vm); return;}

    //comparisions
    if (strcmp(operator, ">") == 0)  {emitByte(OP_GREATER, chunk, parser, vm);       return;}
    if (strcmp(operator, "<") == 0)  {emitByte(OP_LESS, chunk, parser, vm);          return;}
    if (strcmp(operator, ">=") == 0) {emitByte(OP_GREATER_EQUAL, chunk, parser, vm); return;}
    if (strcmp(operator, "<=") == 0) {emitByte(OP_LESS_EQUAL, chunk, parser, vm);    return;}
    if (strcmp(operator, "==") == 0) {emitByte(OP_EQUAL, chunk, parser, vm);         return;}
    if (strcmp(operator, "!=") == 0) {emitByte(OP_NOT_EQUAL, chunk, parser, vm);     return;}

    error("Invalid operation found in one of your math operations", "SYNTAX ERROR", parser);
}
static void emitUnaryOperator(char operator, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    switch (operator)
    {
        case '-': emitByte(OP_NEGATE,  chunk, parser, vm); break;
        case '!': emitByte(OP_INVERSE, chunk, parser, vm); break;
        default:
        {
            error("Invalid unary operator found in expressions, please double check what you are doing.", "SYNTAX ERROR", parser);
            return;
        }
    }
    //hopefully unreachable
}

//variables
void emitDefineGlobal(const char* name, int length, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    ObjString* varName = copyString(name, length, vm);
    uint8_t index = addConstant(chunk, CREATE_OBJECT_VAL((Obj*)varName), vm);
    emitBytes(OP_DEFINE_GLOBAL, index, chunk, parser, vm);
}
void emitGetGlobal(const char* name, int length, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    ObjString* varName = copyString(name, length, vm);
    uint8_t index = addConstant(chunk, CREATE_OBJECT_VAL((Obj*)varName), vm);
    emitBytes(OP_GET_GLOBAL, index, chunk, parser, vm);
}
void emitSetGlobal(const char* name, int length, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    ObjString* varName = copyString(name, length, vm);
    uint8_t index = addConstant(chunk, CREATE_OBJECT_VAL((Obj*)varName), vm);
    emitBytes(OP_SET_GLOBAL, index, chunk, parser, vm);
}
int resolveLocal(AstCompiler* compiler, const char* name, int length)
{
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local* local = &compiler->locals[i];
        if (local->length == length && memcmp(local->name, name, length) == 0) return i; //found right local
    }
    return -1; //no local found, must be global
}
void emitGetLocal(int position, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    emitBytes(OP_GET_LOCAL, (uint8_t)position, chunk, parser, vm);
}

//control flow
short emitJump(uint8_t instruction, Chunk* chunk, ASTparser* parser, Vm* vm)
{
    emitByte(instruction, chunk, parser, vm);
    emitByte(0xff, chunk, parser, vm);
    emitByte(0xff, chunk, parser, vm);
    return (short)(chunk->count - 2);
}

//----------------------------------------Actual compiler stuff-------------------------------------------------//

void compileExpressionByte(Expr* expr, ASTparser* parser, Chunk* vmChunk, AstCompiler* compiler, Vm* vm)
{

    #ifdef DEBUG_TRACE_EXECUTION
        printf("Ran inside of compile expression byte for expression: ");
        printExpression(expr);
        printf("\n");
    #endif

    if (expr == NULL)
    {
        error("NULL expression reached in compiler", "GOD HELP YOU", parser);
        return;
    }

    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            compileExpressionByte(expr->binary.left, parser, vmChunk, compiler, vm);
            compileExpressionByte(expr->binary.right, parser, vmChunk,compiler, vm);
            emitBinaryOperator(expr->binary.operator, vmChunk, parser, vm);
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
            compileExpressionByte(expr->unary.right, parser, vmChunk, compiler, vm);
            emitUnaryOperator(expr->unary.operator, vmChunk, parser, vm);
            break;
        }
        case EXPR_GROUPING:
        {
            //just pass it on
            compileExpressionByte(expr->grouping.expr, parser, vmChunk, compiler, vm);
            break;
        }
        case EXPR_VARIABLE:
        {
            int scan = resolveLocal(compiler, expr->variable.name, expr->variable.length);
            if (scan != -1)
            {

                emitGetLocal(scan, vmChunk, parser, vm);
            }
            else
            {
                int upVal = resolveUpvalue(compiler, expr->variable.name, expr->variable.length);                  //look for upval
                if (upVal != -1) { emitBytes(OP_GET_UPVALUE, (uint8_t)upVal, vmChunk, parser, vm); }   //emit getting it to the vm
                else {emitGetGlobal(expr->variable.name, expr->variable.length, vmChunk, parser, vm); }            //emit global as a final resort                                                  //otherwise you know it is a local
            }
            break;
        }
        case EXPR_ASSIGN:
        {
            compileExpressionByte(expr->var_assignment.value, parser, vmChunk, compiler, vm);
            int slot = resolveLocal(compiler, expr->var_assignment.name, expr->var_assignment.length);
            if (slot != -1)
            {
                emitBytes(OP_SET_LOCAL, (uint8_t)slot, vmChunk, parser, vm);
            }
            else
            {
                int upVal = resolveUpvalue(compiler, expr->var_assignment.name, expr->var_assignment.length);                  //look for upval
                if (upVal != -1) { emitBytes(OP_SET_UPVALUE, (uint8_t)upVal, vmChunk, parser, vm); }   //emit getting it to the vm
                else {emitSetGlobal(expr->var_assignment.name, expr->var_assignment.length, vmChunk, parser, vm);} //otherwise just set the local var
            }
            break;
        }
        case EXPR_CALL:
        {
            compileExpressionByte(expr->objectCall.callee, parser, vmChunk, compiler, vm);
            for (int i = 0; i < expr->objectCall.argCount; i++)
            {
                compileExpressionByte(expr->objectCall.args[i], parser, vmChunk, compiler, vm);
            }
            emitBytes(OP_CALL, (uint8_t)expr->objectCall.argCount, vmChunk, parser, vm);
            break;
        }
        case EXPR_STATIC_ARRAY:
        {
            for (int i = 0; i < expr->staticArray.length; i++)
            {
                compileBytecode(expr->staticArray.values[i], parser, vmChunk, compiler, vm);
            }
            // emit the count as an operand
            emitByte(OP_CREATE_ARRAY, vmChunk, parser, vm);
            emitByte(expr->staticArray.length, vmChunk, parser, vm);
            emitByte((uint8_t)expr->staticArray.type, vmChunk, parser, vm);
            break;
        }
        case EXPR_GET_ARRAY_INDEX:
        {
            compileBytecode(expr->getArray.left, parser, vmChunk, compiler, vm);
            compileBytecode(expr->getArray.index, parser, vmChunk, compiler, vm);
            emitByte(OP_GET_ARRAY_INDEX, vmChunk, parser, vm);
            break;
        }
        case EXPR_SET_ARRAY_INDEX:
        {
            compileBytecode(expr->setArray.left, parser, vmChunk, compiler, vm);
            compileBytecode(expr->setArray.index, parser, vmChunk, compiler, vm);
            compileBytecode(expr->setArray.value, parser, vmChunk, compiler, vm);
            emitByte(OP_SET_ARRAY_INDEX, vmChunk, parser, vm);
            break;
        }
        case EXPR_AND:
        {
            compileExpressionByte(expr->andExpr.left, parser, vmChunk, compiler, vm);
            int endJump = emitJump(OP_JUMP_IF_FALSE, vmChunk, parser, vm);
            emitByte(OP_POP, vmChunk, parser, vm);
            compileExpressionByte(expr->andExpr.right, parser, vmChunk, compiler, vm);
            patchJump(endJump, vmChunk, parser);
            break;
        }
        case EXPR_OR:
        {
            compileExpressionByte(expr->orExpr.left, parser, vmChunk, compiler, vm);
            int elseJump = emitJump(OP_JUMP_IF_FALSE, vmChunk, parser, vm);
            int endJump  = emitJump(OP_JUMP, vmChunk, parser, vm);
            patchJump(elseJump, vmChunk, parser);
            emitByte(OP_POP, vmChunk, parser, vm);
            compileExpressionByte(expr->orExpr.right, parser, vmChunk, compiler, vm);
            patchJump(endJump, vmChunk, parser);
            break;
        }

    }
}


void compileBytecode(Expr* expr, ASTparser* parser, Chunk* vmChunk, AstCompiler* compiler, Vm* vm)
{
    //TODO: add in a way to seperate consequtive expressions from return call
    compileExpressionByte(expr, parser, vmChunk, compiler, vm);
}
