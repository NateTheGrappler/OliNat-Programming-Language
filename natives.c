//
// Created by natang on 6/14/26.
//
#include "natives.h"

//-----------------io natives--------------------//
Value printNative(int argCount, Value* args)
{
    printValue(args[0]);
    printf("\n");
    fflush(stdout);
    return CREATE_EMPTY_VAL(); //TODO: handle void returns better
}
