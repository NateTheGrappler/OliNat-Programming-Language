//
// Created by natang on 6/14/26.
//
#include "natives.h"

//-----------------io natives--------------------//
Value printNative(int argCount, Value* args, struct Vm* vm)
{
    printValue(args[0]);
    return CREATE_EMPTY_VAL();
}
Value printlnNatve(int argCount, Value* args, struct Vm* vm)
{
    printValue(args[0]);
    printf("\n");
    fflush(stdout);
    return CREATE_EMPTY_VAL();
}
Value intakeNative(int argCount, Value* args, struct Vm* vm)
{
    if (argCount > 0 && IS_STRING(args[0])) { printf("%s", AS_CSTRING(args[0])); fflush(stdout); } //print prompt if one

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        //error
        ObjString* emptyString = copyString("", 0, vm);
        return CREATE_OBJECT_VAL((Obj*)emptyString);
    }

    int length = (int)strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0';
        length--;
    }

    ObjString* result = copyString(buffer, length, vm);
    return CREATE_OBJECT_VAL((Obj*)result);
}

//--------------math natives----------------//
Value sinNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL(sin(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL(sin(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(sin(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();

}
Value cosNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL(cos(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL(cos(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(cos(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value tanNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL(tan(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL(tan(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(tan(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value absNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL((double)abs(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL((double)fabsf(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(fabs(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value logNative(int argCount, Value* args, struct Vm* vm)
{

}
Value sqrtNative(int argCount, Value* args, struct Vm* vm)
{

}
Value floorNative(int argCount, Value* args, struct Vm* vm)
{

}
Value ceilNative(int argCount, Value* args, struct Vm* vm)
{

}
Value expoNative(int argCount, Value* args, struct Vm* vm)
{

}

//--------------Random natives----------------// //TODO: add random

//--------------time natives----------------//
Value clockNative(int argCount, Value* args, struct Vm* vm) //mark time, returns double
{
    return CREATE_DOUBLE_VAL((double)clock() / CLOCKS_PER_SEC);
}
Value sleepNative(int argCount, Value* args, struct Vm* vm) //pause program for x number of milliseconds
{

}
Value timeNative(int argCount, Value* args, struct Vm* vm)  //Unix timestamp as a double
{

}
Value dateStringNative(int argCount, Value* args, struct Vm* vm) //print out date as string "06-15-2077"
{

}
Value timeStringNative(int argCount, Value* args, struct Vm* vm) //print out time of day as string "14:30:00"
{

}

//--------------fileIO natives----------------//

Value readFileNative(int argCount, Value* args, struct Vm* vm) //get all content from file
{

}
//Value readLineFileNative(int argCount, Value* args, struct Vm* vm);  //read only one line in the file //TODO: maybe add these later with some state
//Value readWordFileNative(int argCount, Value* args, struct Vm* vm);  //read only one word in the file //TODO: maybe add these later with some state
Value writeFileNative(int argCount, Value* args, struct Vm* vm)     //create a new file and then write to it
{

}
Value appendFileNative(int argCount, Value* args, struct Vm* vm)    //write onto the file, append to it
{

}
Value fileExistsNative(int argCount, Value* args, struct Vm* vm)    //bool to see if file exists at path
{

}
Value deleteFileNative(int argCount, Value* args, struct Vm* vm)    //delete file at path
{

}

//--------------Type natives----------------//  //TODO: update this if I ever add longs and shorts
Value intToStrNative(int argCount, Value* args, struct Vm* vm)
{

}
Value intToDoubleNative(int argCount, Value* args, struct Vm* vm)
{

}
Value intToFloatNative(int argCount, Value* args, struct Vm* vm)
{

}

Value doubleToStrNative(int argCount, Value* args, struct Vm* vm)
{

}
Value doubleToIntNative(int argCount, Value* args, struct Vm* vm)
{

}
Value doubleToFloatNative(int argCount, Value* args, struct Vm* vm)
{

}

Value floatToStrNative(int argCount, Value* args, struct Vm* vm)
{

}
Value floatToIntNative(int argCount, Value* args, struct Vm* vm)
{

}
Value floatToDoubleNative(int argCount, Value* args, struct Vm* vm)
{

}

Value strToFloatNative(int argCount, Value* args, struct Vm* vm)
{

}
Value strToIntNative(int argCount, Value* args, struct Vm* vm)
{

}
Value strToDoubleNative(int argCount, Value* args, struct Vm* vm)
{

}
Value strToBoolNative(int argCount, Value* args, struct Vm* vm)
{

}

Value boolToStrNative(int argCount, Value* args, struct Vm* vm)
{

}

//--------------hashMap natives----------------//
//TODO: do later
//--------------dynamic array natives----------------//
//TODO: do later

//--------------utils natives----------------//
Value lengthNative(int argCount, Value* args, struct Vm* vm)
{

}
Value assertNative(int argCount, Value* args, struct Vm* vm)
{

}
