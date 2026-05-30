//
// Created by natang on 5/29/26.
//
#include "scanner.h"
#include "common.h"


//init function for scanner
void initScanner(const char* source, Scanner* scanner)
{
    scanner->current = source;
    scanner->start = source;
    scanner->line = 1;
}

//---------------------Helper Functions-------------------------//

static bool isAtEnd(Scanner* scanner)
{
    return *scanner->current == '\0';
}
static bool match(char expected, Scanner* scanner)
{
    if (isAtEnd(scanner)) return false;
    if (*scanner->current != expected) return false;

    //if found, consume char and return true
    scanner->current++;
    return true;
}

static Token makeToken(TokenType type, Scanner* scanner)
{
    Token token;
    token.type = type;
    token.line = scanner->line;
    token.lexemeStart = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    return token;
}
static Token errorToken(const char* message, Scanner* scanner)
{
    //basically makeToken but a little different for errors
    Token token;
    token.type = T_ERROR;
    token.lexemeStart = message;
    token.length = (int)strlen(message);
    token.line = scanner->line;
    return token;
}

static char advance(Scanner* scanner)
{
    //return current token and advance to next one
    return *scanner->current++;
}
static char peekChar(Scanner* scanner)
{
    return *scanner->current;
}
static char peekNextChar(Scanner* scanner)
{
    if (isAtEnd(scanner)) return '\0';
    return scanner->current[1];

}

//functions that make stuff that isnt basic
static Token string(Scanner* scanner)
{
    return makeToken(T_STRING, scanner);
}

//---------------------Actual like, meat function-------------------------//

Token scanToken(Scanner* scanner)
{
    //TODO: add whitespace skipping

    scanner->start = scanner->current;

    if (isAtEnd(scanner)) {return makeToken(T_EOF, scanner); }

    char c = advance(scanner);
    printf("Character getting scanner: '%c'.", c);

    //TODO: add handlings for numbers and strings

    switch (c)
    {
        //single char tokens
        case '(': return makeToken(T_LEFT_PAREN,  scanner);
        case ')': return makeToken(T_RIGHT_PAREN, scanner);
        case '{': return makeToken(T_LEFT_BRACE,  scanner);
        case '}': return makeToken(T_RIGHT_BRACE, scanner);
        case ';': return makeToken(T_SEMICOLON,   scanner);
        case ',': return makeToken(T_COMMA,       scanner);
        case '.': return makeToken(T_DOT,         scanner);


        //possibly single or double tokens
        case '*': return makeToken(match('*', scanner) ? T_STAR_EQUAL    : T_STAR,    scanner);
        case '+': return makeToken(match('+', scanner) ? T_PLUS_EQUAL    : T_PLUS,    scanner);
        case '-': return makeToken(match('-', scanner) ? T_MINUS_EQUAL   : T_MINUS,   scanner);
        case '/': return makeToken(match('/', scanner) ? T_SLASH_EQUAL   : T_SLASH,   scanner);
        case '=': return makeToken(match('=', scanner) ? T_EQUAL_EQUAL   : T_EQUAL,   scanner);
        case '!': return makeToken(match('!', scanner) ? T_BANG_EQUAL    : T_BANG,    scanner);
        case '>': return makeToken(match('>', scanner) ? T_GREATER_EQUAL : T_GREATER, scanner);
        case '<': return makeToken(match('<', scanner) ? T_LESS_EQUAL    : T_LESS,    scanner);

        //handle strings
        case '"': return string(scanner);
    }

    //base return case for error tokens
    return errorToken("An unexpected character was encountered, what on earth are you typing?", scanner);
}