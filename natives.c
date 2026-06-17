//
// Created by natang on 6/14/26.
//
#include "natives.h"
#include "vm.h"

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
Value log10Native(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL(log10(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL(log10(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(log10(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value log2Native(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL(log2(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL(log2(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(log2(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value lnNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return  CREATE_DOUBLE_VAL(log(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL(log(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(log(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value sqrtNative(int argCount, Value* args, struct Vm* vm)
{
    double x;
    if (IS_INT(args[0]))        x = (double)GET_INT_VAL(args[0]);
    else if (IS_FLOAT(args[0])) x = (double)GET_FLOAT_VAL(args[0]);
    else                        x = GET_DOUBLE_VAL(args[0]);

    if (x < 0)
    {
        runtimeError(vm, "sqrt() requires a non-negative number input, no imagination in my programming language.\n", "LOGIC ERROR");
        return CREATE_EMPTY_VAL();
    }
    return CREATE_DOUBLE_VAL(sqrt(x));
}
Value floorNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return CREATE_DOUBLE_VAL(floor(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL((double)floorf(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(floor(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value ceilNative(int argCount, Value* args, struct Vm* vm)
{

    if (IS_INT(args[0]))    { return CREATE_DOUBLE_VAL(ceil(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL((double)ceilf(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(ceil(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value expoNative(int argCount, Value* args, struct Vm* vm)
{
    if (IS_INT(args[0]))    { return CREATE_DOUBLE_VAL(exp(GET_INT_VAL(args[0]))); }
    if (IS_FLOAT(args[0]))  { return CREATE_DOUBLE_VAL((double)expf(GET_FLOAT_VAL(args[0]))); }
    if (IS_DOUBLE(args[0])) { return CREATE_DOUBLE_VAL(exp(GET_DOUBLE_VAL(args[0]))); }
    return CREATE_EMPTY_VAL();
}
Value powNative(int argCount, Value* args, struct Vm* vm)
{
    Value a = args[0]; //base
    Value b = args[1]; //powere
    if (IS_INT(a) && IS_INT(b))
    {
        return CREATE_DOUBLE_VAL(pow(GET_INT_VAL(a), GET_INT_VAL(b)));
    }
    if (IS_DOUBLE(a) || IS_DOUBLE(b))
    {
        double da = IS_DOUBLE(a) ? GET_DOUBLE_VAL(a) : (double)GET_INT_VAL(a);
        double db = IS_DOUBLE(b) ? GET_DOUBLE_VAL(b) : (double)GET_INT_VAL(b);
        return CREATE_DOUBLE_VAL(pow(da, db));
    }
    if (IS_FLOAT(a) || IS_FLOAT(b))
    {
        double fa = IS_FLOAT(a) ? GET_FLOAT_VAL(a) : (double)GET_INT_VAL(a);
        double fb = IS_FLOAT(b) ? GET_FLOAT_VAL(b) : (double)GET_INT_VAL(b);
        return CREATE_DOUBLE_VAL(pow(fa, fb));
    }
    return CREATE_EMPTY_VAL();
}

//--------------Random natives----------------// //TODO: add random

//--------------time natives----------------//
Value clockNative(int argCount, Value* args, struct Vm* vm) //mark time, returns double
{
    return CREATE_DOUBLE_VAL((double)clock() / CLOCKS_PER_SEC);
}
Value sleepNative(int argCount, Value* args, struct Vm* vm) //pause program for x number of milliseconds
{
    double ms = 0;
    if (IS_INT(args[0]))    { ms = (double)GET_INT_VAL(args[0]); }
    if (IS_FLOAT(args[0]))  { ms = (double)GET_FLOAT_VAL(args[0]); }
    if (IS_DOUBLE(args[0])) { ms = GET_DOUBLE_VAL(args[0]); }

#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)((ms - (ts.tv_sec * 1000)) * 1000000);
    nanosleep(&ts, NULL);
#endif

    return CREATE_EMPTY_VAL();
}
Value timeNative(int argCount, Value* args, struct Vm* vm)  //Unix timestamp as a double (seconds since January 1, 1970)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0 )
    {
        runtimeError(vm, "Error reading system clock value in native time function.", "INTERNAL ERROR");
        return CREATE_EMPTY_VAL();
    }

    return CREATE_DOUBLE_VAL((double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0));
}
Value dateStringNative(int argCount, Value* args, struct Vm* vm) //print out date as string "06-15-2077"
{
    time_t rawTime = time(NULL);

    struct tm *localTime = localtime(&rawTime);
    char dateString[100];
    strftime(dateString, sizeof(dateString), "%m-%d-%Y", localTime);
    ObjString* returnString = copyString(dateString, (int)strlen(dateString), vm);
    return CREATE_OBJECT_VAL((Obj*)returnString);
}
Value timeStringNative(int argCount, Value* args, struct Vm* vm) //print out time of day as string "14:30:00"
{
    time_t rawTime = time(NULL);

    struct tm *localTime = localtime(&rawTime);
    char dateString[100];
    strftime(dateString, sizeof(dateString), "%H:%M:%S", localTime);
    ObjString* returnString = copyString(dateString, (int)strlen(dateString), vm);
    return CREATE_OBJECT_VAL((Obj*)returnString);
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
    char str[12];
    snprintf(str, sizeof(str), "%d", GET_INT_VAL(args[0]));
    ObjString* returnString = copyString(str, (int)strlen(str), vm);
    return CREATE_OBJECT_VAL((Obj*)returnString);
}
Value intToDoubleNative(int argCount, Value* args, struct Vm* vm)
{
    return CREATE_DOUBLE_VAL((double)GET_INT_VAL(args[0]));
}
Value intToFloatNative(int argCount, Value* args, struct Vm* vm)
{
    return CREATE_FLOAT_VAL((float)GET_INT_VAL(args[0]));
}

Value doubleToStrNative(int argCount, Value* args, struct Vm* vm)
{
    char str[50];
    snprintf(str, sizeof(str), "%f", GET_DOUBLE_VAL(args[0]));
    ObjString* returnString = copyString(str, (int)strlen(str), vm);
    return CREATE_OBJECT_VAL((Obj*)returnString);
}
Value doubleToIntNative(int argCount, Value* args, struct Vm* vm)
{
    return CREATE_INT_VAL((int)GET_DOUBLE_VAL(args[0]));
}
Value doubleToFloatNative(int argCount, Value* args, struct Vm* vm)
{
    return CREATE_FLOAT_VAL((float)GET_DOUBLE_VAL(args[0]));
}

Value floatToStrNative(int argCount, Value* args, struct Vm* vm)
{
    char str[20];
    snprintf(str, sizeof(str), "%f", GET_FLOAT_VAL(args[0]));
    ObjString* returnString = copyString(str, (int)strlen(str), vm);
    return CREATE_OBJECT_VAL((Obj*)returnString);
}
Value floatToIntNative(int argCount, Value* args, struct Vm* vm)
{
    return CREATE_INT_VAL((int)GET_FLOAT_VAL(args[0]));
}
Value floatToDoubleNative(int argCount, Value* args, struct Vm* vm)
{
    return CREATE_DOUBLE_VAL((double)GET_FLOAT_VAL(args[0]));
}

Value strToFloatNative(int argCount, Value* args, struct Vm* vm)
{
    char* str = AS_CSTRING(args[0]);
    char* end;
    errno = 0;
    float result = strtof(str, &end);

    if (end == str || *end != '\0' || errno == ERANGE)
    {
        runtimeError(vm, "Unable to turn given string into a float value.", "TYPE ERROR");
        return CREATE_EMPTY_VAL();
    }
    return CREATE_FLOAT_VAL(result);
}
Value strToIntNative(int argCount, Value* args, struct Vm* vm)
{
    char* str = AS_CSTRING(args[0]);
    char* end;
    errno = 0;
    long result = strtol(str, &end, 10);

    if (end == str || *end != '\0' || errno == ERANGE)
    {
        runtimeError(vm, "Unable to turn given string into an int value.", "TYPE ERROR");
        return CREATE_EMPTY_VAL();
    }
    return CREATE_INT_VAL((int)result);
}
Value strToDoubleNative(int argCount, Value* args, struct Vm* vm)
{
    char* str = AS_CSTRING(args[0]);
    char* end;
    errno = 0;
    double result = strtod(str, &end);

    if (end == str || *end != '\0' || errno == ERANGE)
    {
        runtimeError(vm, "Unable to turn given string into a double value.", "TYPE ERROR");
        return CREATE_EMPTY_VAL();
    }
    return CREATE_DOUBLE_VAL(result);
}
Value strToBoolNative(int argCount, Value* args, struct Vm* vm)
{
    char* str = AS_CSTRING(args[0]);

    if (strcmp(str, "true") == 0)  return CREATE_BOOL_VAL(true);
    if (strcmp(str, "false") == 0) return CREATE_BOOL_VAL(false);

    runtimeError(vm, "Cannot convert string to bool: expected 'true' or 'false'.", "TYPE ERROR");
    return CREATE_EMPTY_VAL();
}

Value boolToStrNative(int argCount, Value* args, struct Vm* vm)
{
    bool tOrF = GET_BOOL_VAL(args[0]);

    if (tOrF)
    {
        const char* a = "true";
        ObjString* returnString = copyString(a, 4, vm);
        return CREATE_OBJECT_VAL((Obj*)returnString);
    }
    else
    {
        const char* b = "false";
        ObjString* returnString = copyString(b, 5, vm);
        return CREATE_OBJECT_VAL((Obj*)returnString);
    }
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
