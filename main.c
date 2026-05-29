#include "common.h"

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
    if (sourceBuffer == NULL) {}
    size_t bytesRead = fread(sourceBuffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {}
    sourceBuffer[0] = '\0'; //add EOF char

    fclose(file);
    return sourceBuffer;
}

//handle the reference to getting the info from the file and
//passing it to rest of vm
static void runFile(const char* filePath)
{
    const char* source = readFile(filePath);
    printf("%s\n", source);
}


int main(int argc, const char* argv[])
{
    if (argc == 1)
    {
        //take in code from repl
        repl();
    }
    else if (argc == 2)
    {
        //start running the code from the passed in file
        runFile(argv[1]);
    }
    else
    {
        //incorrect usage of executable
        fprintf(stderr, "Please pass in either one file path, or no file paths.");
    }
    return 0;
}
