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
static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}
static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' &&  c<='Z') || (c == '_') || (c == '#');
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

//functions that make stuff that isn't basic
static Token string(Scanner* scanner)
{
    while (peekChar(scanner) != '"' && !isAtEnd(scanner))
    {
        if (peekChar(scanner) == '\n') scanner++;
        advance(scanner);
    }

    if (isAtEnd(scanner)) return errorToken("Must use a double quote to end your string.", scanner);
    advance(scanner);
    return makeToken(T_STRING_VAL, scanner);
}
static Token number(Scanner* scanner)
{

    //base int stuff
    while (isDigit(peekChar(scanner)))
    {
        advance(scanner);
    }

    //check for deecimal values
    if (peekChar(scanner) == '.' && isDigit(peekNextChar(scanner)))
    {
        //consume decimal values
        advance(scanner);
        while (isDigit(peekChar(scanner)))
        {
            advance(scanner);
        }

        //check to see for a floating point value
        if (peekChar(scanner) == 'f')
        {
            advance(scanner); //consume 'f' char
            return makeToken(T_FLOAT_VAL, scanner);
        }

        //if no floats just toss out double
        return makeToken(T_DOUBLE_VAL, scanner);
    }


    return  makeToken(T_INTEGER_VAL, scanner);

}

//use the tree method for checking for keywords
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type, Scanner* scanner)
{
    if (scanner->current - scanner->start == start + length && memcmp(scanner->start + start, rest, length) == 0)
    {
        return type;
    }
    return T_IDENTIFIER;
}
static TokenType identifierType(Scanner* scanner)
{
    switch (scanner->start[0])
    {
        //only the starting letters matter
        case 'a' : return checkKeyword(1, 2, "nd",    T_AND, scanner);
        case 'b' : return checkKeyword(1, 3, "ool",   T_BOOL, scanner);
        case 'c' : return checkKeyword(1, 4, "lass",  T_CLASS, scanner);
        case 'd' : return checkKeyword(1, 5, "ouble", T_DOUBLE, scanner);
        case 'm' : return checkKeyword(1, 3, "ake",   T_MAKE, scanner);
        case 'o' : return checkKeyword(1, 1, "r",     T_OR, scanner);
        case 'p' : return checkKeyword(1, 4, "ullf",  T_PULLF, scanner);
        case 'r' : return checkKeyword(1, 5, "eturn", T_RETURN, scanner);
        case 's' : return checkKeyword(1, 5, "tring", T_STRING, scanner);
        case 'w' : return checkKeyword(1, 4, "hile",  T_WHILE, scanner);
        case '#' : return checkKeyword(1, 5, "pullf", T_HASH_PULLF, scanner);

        //cases for words with branching letters
        case 'e':
        {
            if (scanner->current - scanner->start > 1)
            {
                switch (scanner->start[1])
                {
                    case 'l': return checkKeyword(2, 2, "se", T_ELSE, scanner);
                    case 'm': return checkKeyword(2, 3, "pty", T_EMPTY, scanner);
                }
            }
            return T_IDENTIFIER;
        }
        case 'i':
        {
            if (scanner->current - scanner->start > 1)
            {
                switch (scanner->start[1])
                {
                    case 'f': return checkKeyword(2, 0, "", T_IF, scanner);
                    case 'n':
                    {
                        //holy nested doom
                        if (scanner->current - scanner->start > 2)
                        {
                            switch (scanner->start[2])
                            {
                                case 'h': return checkKeyword(3, 4, "erit", T_INHERIT, scanner);
                                case 't': return checkKeyword(3, 0, "", T_INTEGER, scanner); //int keyword
                            }
                        }
                    }
                }
            }
            return T_IDENTIFIER;
        }
        case 't':
        {
            if (scanner->current - scanner->start > 1)
            {
                switch (scanner->start[1])
                {
                    case 'h': return checkKeyword(2, 2, "is", T_THIS, scanner);
                    case 'r': return checkKeyword(2, 2, "ue", T_TRUE, scanner);

                }
            }
            return T_IDENTIFIER;
        }
        case 'f':
        {
            if (scanner->current - scanner->start > 1)
            {
                switch (scanner->start[1])
                {
                    case 'o': return checkKeyword(2, 1, "r", T_FOR, scanner);
                    case 'u': return checkKeyword(2, 1, "n", T_FUN, scanner);
                    case 'a': return checkKeyword(2, 3, "lse", T_FALSE, scanner);
                    case 'l': return checkKeyword(2, 3, "oat", T_FLOAT, scanner);
                }
            }
            return T_IDENTIFIER;
        }
        default:
            return T_IDENTIFIER;

    }
}
static Token identifier(Scanner* scanner)
{
    //consume the identifier so it is stored in the scanner
    while (isAlpha(peekChar(scanner)) || isDigit(peekChar(scanner)))
    {
        advance(scanner);
    }

    //find out if it's a keyword or just an identifier
    return makeToken(identifierType(scanner), scanner);
}

//skipping all that whitespace nonsense
static void skipWhiteSpace(Scanner* scanner)
{
    for (;;)
    {
        //printf("stuck in loop");
        char c = peekChar(scanner);
        switch (c)
        {
            case ' ':   //regular spaces
            case '\r':  //moves cursor to column one
            case '\t':  //tabs
                advance(scanner);
                break;
            case '\n':  //newline
                scanner->line++;
                advance(scanner);
                break;
            //comments
            case '/':
                if (peekNextChar(scanner) == '/')
                {
                    while (peekChar(scanner) != '\n' && !isAtEnd(scanner)) advance(scanner);
                    break;
                }
                return;
            default: return;
        }
    }
}
//---------------------Actual like, meat function-------------------------//

Token scanToken(Scanner* scanner)
{
    skipWhiteSpace(scanner);

    scanner->start = scanner->current;

    if (isAtEnd(scanner)) {return makeToken(T_EOF, scanner); }

    char c = advance(scanner);

    if (isDigit(c)) return number(scanner);
    if (isAlpha(c)) return identifier(scanner);
    //printf("Character getting scanner: '%c'.", c);

    switch (c)
    {
        //single char tokens
        case '(': return makeToken(T_LEFT_PAREN,  scanner);
        case ')': return makeToken(T_RIGHT_PAREN, scanner);
        case '{': return makeToken(T_LEFT_BRACE,  scanner);
        case '}': return makeToken(T_RIGHT_BRACE, scanner);
        case '[': return makeToken(T_LEFT_BRACKET,  scanner);
        case ']': return makeToken(T_RIGHT_BRACKET, scanner);
        case ';': return makeToken(T_SEMICOLON,   scanner);
        case ',': return makeToken(T_COMMA,       scanner);
        case '.': return makeToken(T_DOT,         scanner);


        //possibly single or double tokens
        case '*': return makeToken(match('=', scanner) ? T_STAR_EQUAL    : T_STAR,    scanner);
        case '/': return makeToken(match('=', scanner) ? T_SLASH_EQUAL   : T_SLASH,   scanner);
        case '=': return makeToken(match('=', scanner) ? T_EQUAL_EQUAL   : T_EQUAL,   scanner);
        case '!': return makeToken(match('=', scanner) ? T_BANG_EQUAL    : T_BANG,    scanner);
        case '>': return makeToken(match('=', scanner) ? T_GREATER_EQUAL : T_GREATER, scanner);
        case '<': return makeToken(match('=', scanner) ? T_LESS_EQUAL    : T_LESS,    scanner);
        case '|': if (match('|', scanner)) return makeToken(T_OR, scanner);
        case '&': if (match('&', scanner)) return makeToken(T_AND, scanner);
        case '+':
        {
            if (match('+', scanner)) return makeToken(T_PLUS_PLUS, scanner);
            if (match('=', scanner)) return makeToken(T_PLUS_EQUAL, scanner);
            return makeToken(T_PLUS, scanner);
        }
        case '-':
        {
            if (match('-', scanner)) return makeToken(T_MINUS_MINUS, scanner);
            if (match('=', scanner)) return makeToken(T_MINUS_EQUAL, scanner);
            return makeToken(T_MINUS, scanner);
        }
        //handle strings
        case '"': return string(scanner); //TODO: see if you can add chars
    }

    //base return case for error tokens
    return errorToken("An unexpected character was encountered, what on earth are you typing?", scanner);

}