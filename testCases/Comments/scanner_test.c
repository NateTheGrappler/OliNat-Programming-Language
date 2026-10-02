#include <assert.h>
#include <string.h>
#include "scanner.h"

static void expect(Scanner* scanner, TokenType type, int line, const char* text)
{
    Token token = scanToken(scanner);
    assert(token.type == type);
    assert(token.line == line);
    assert(token.length == (int)strlen(text));
    assert(memcmp(token.lexemeStart, text, token.length) == 0);
}

int main(void)
{
    Scanner scanner;
    initScanner("/**//* first\nsecond */make/* inline */ int x = 8 / 2;", &scanner);
    expect(&scanner, T_MAKE, 2, "make");
    expect(&scanner, T_INTEGER, 2, "int");
    expect(&scanner, T_IDENTIFIER, 2, "x");
    expect(&scanner, T_EQUAL, 2, "=");
    expect(&scanner, T_INTEGER_VAL, 2, "8");
    expect(&scanner, T_SLASH, 2, "/");
    expect(&scanner, T_INTEGER_VAL, 2, "2");
    expect(&scanner, T_SEMICOLON, 2, ";");
    expect(&scanner, T_EOF, 2, "");

    initScanner("// /* line comment\n/* // ignored\n**/ /= \"/* string */\"", &scanner);
    expect(&scanner, T_SLASH_EQUAL, 3, "/=");
    expect(&scanner, T_STRING_VAL, 3, "\"/* string */\"");
    expect(&scanner, T_EOF, 3, "");

    initScanner("/* comment */", &scanner);
    expect(&scanner, T_EOF, 1, "");

    initScanner("/* unfinished\n*", &scanner);
    expect(&scanner, T_ERROR, 2, "Unterminated block comment.");
    expect(&scanner, T_EOF, 2, "");

    initScanner("/*", &scanner);
    expect(&scanner, T_ERROR, 1, "Unterminated block comment.");
    expect(&scanner, T_EOF, 1, "");
    return 0;
}
