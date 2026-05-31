#include "common.h"
#include "vm.h"
#include "Expr.h"
#include "debug.h"

//start repl loop
static void repl()
{
    //TODO: add in while loop for repl and what not
}

//reading from files, return the source code char array
static const char* readFile(const char* filepath)
{
    FILE* file = fopen(filepath, "rb");

    //handle being unable to read a file
    if (file == NULL)
    {
        fprintf(stderr, "Unable to read file: \"%s\".\n", filepath);
        exit(74);
    }

    //seek to the end of the file, set aside that file size, and then go back to start of file
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    //set aside enough memory to store the file and then toss it up in there
    char* sourceBuffer = (char*)malloc(fileSize + 1);
    if (sourceBuffer == NULL) { fprintf(stderr, "Not enough memory to read \"%s\".\n", filepath); exit(74); }
    size_t bytesRead = fread(sourceBuffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {fprintf(stderr, "Could not read file \"%s\".\n", filepath); exit(74); }
    sourceBuffer[fileSize] = '\0'; //add EOF char

    fclose(file);
    return sourceBuffer;
}

//handle the reference to getting the info from the file and
//passing it to rest of vm
static void runFile(const char* filePath, Vm* vm)
{
    //get a char array of the source
    const char* source = readFile(filePath);

    //test out if expressions and printing are working
    //(5+3)+2

    Expr* five = createLiteralInt(5);
    Expr* three = createLiteralInt(3);
    Expr* two = createLiteralInt(2);
    Expr* add = createBinary(five, three, '+');
    Expr* group = createGrouping(add);
    Expr* multiply = createBinary(group, two, '*');

    printExpression(multiply);

    // Expr* add = createBinary(createLiteralInt(1), createLiteralInt(2), '+');
    // printExpression(add);
}


int main(int argc, const char* argv[])
{
    Vm vm;
    initVM(&vm);

    if (argc == 1)
    {
        //take in code from repl
        repl();
    }
    else if (argc == 2)
    {
        //start running the code from the passed in file
        runFile(argv[1], &vm);
    }
    else
    {
        //incorrect usage of executable
        fprintf(stderr, "Please pass in either one file path, or no file paths.");
    }

    freeVM(&vm);


    return 0;
}
