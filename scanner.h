//
// Created by natang on 5/29/26.
//

#ifndef OLI_NAT_SCANNER_H
#define OLI_NAT_SCANNER_H

//set up the types of tokens the language accepts

typedef enum
{
    //single char
    T_LEFT_PAREN, T_RIGHT_PAREN,
    T_LEFT_BRACE, T_RIGHT_BRACE,
    T_COMMA, T_DOT, T_MINUS, T_PLUS,
    T_SEMICOLON, T_SLASH, T_STAR,

    //one or two chars
    T_BANG, T_BANG_EQUAL,
    T_EQUAL, T_EQUAL_EQUAL,
    T_GREATER, T_GREATER_EQUAL,
    T_LESS, T_LESS_EQUAL,
    T_OR, T_AND,

    //possibly add support for assignment changes
    T_PLUS_EQUAL, T_MINUS_EQUAL, T_PLUS_PLUS,
    T_STAR_EQUAL, T_SLASH_EQUAL, T_MINUS_MINUS,

    //literals
    T_IDENTIFIER, T_STRING_VAL, T_DOUBLE_VAL,
    T_INTEGER_VAL, T_FLOAT_VAL,

    //keywords
    T_FALSE, T_TRUE, T_MAKE, T_WHILE,
    T_FOR, T_IF, T_ELSE, T_RETURN,
    T_CLASS, T_FUN, T_EMPTY, T_INHERIT,
    T_THIS, T_PULLF,
    T_INTEGER, T_FLOAT, T_DOUBLE, T_STRING, T_BOOL,

    //extras
    T_ERROR, T_EOF

} TokenType;

typedef struct
{
    TokenType type;
    const char* lexemeStart;
    int length;
    int line;
} Token;

//scanner class holding current state in source code string
typedef struct
{
    const char* current; //current char your on
    const char* start;   //starter char for current token
    int line;
} Scanner;


void initScanner(const char* source, Scanner* scanner);
Token scanToken(Scanner* scanner);


#endif //OLI_NAT_SCANNER_H
