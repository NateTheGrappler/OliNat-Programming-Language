//
// Created by natang on 6/14/26.
//

#ifndef OLI_NAT_NATIVES_H
#define OLI_NAT_NATIVES_H
#include "common.h"
#include "values.h"
#include "object.h"
#include "debug.h"
#include <errno.h>
#include <time.h>
#include <bits/time.h>
#include <math.h>

typedef struct Vm;

//-----------------io natives--------------------//
Value printNative(int argCount, Value* args, struct Vm* vm);
Value printlnNatve(int argCount, Value* args, struct Vm* vm);
Value intakeNative(int argCount, Value* args, struct Vm* vm);

//--------------math natives----------------//
Value sinNative(int argCount, Value* args, struct Vm* vm);
Value cosNative(int argCount, Value* args, struct Vm* vm);
Value tanNative(int argCount, Value* args, struct Vm* vm);
Value absNative(int argCount, Value* args, struct Vm* vm);
Value log10Native(int argCount, Value* args, struct Vm* vm);
Value log2Native(int argCount, Value* args, struct Vm* vm);
Value lnNative(int argCount, Value* args, struct Vm* vm);
Value sqrtNative(int argCount, Value* args, struct Vm* vm);
Value floorNative(int argCount, Value* args, struct Vm* vm);
Value ceilNative(int argCount, Value* args, struct Vm* vm);
Value expoNative(int argCount, Value* args, struct Vm* vm);
Value powNative(int argCount, Value* args, struct Vm* vm);

//--------------Random natives----------------// //TODO: add random

//--------------time natives----------------//
Value clockNative(int argCount, Value* args, struct Vm* vm); //mark time, returns double
Value sleepNative(int argCount, Value* args, struct Vm* vm); //pause program for x number of milliseconds
Value timeNative(int argCount, Value* args, struct Vm* vm);  //Unix timestamp as a double
Value dateStringNative(int argCount, Value* args, struct Vm* vm); //print out date as string "06-15-2077"
Value timeStringNative(int argCount, Value* args, struct Vm* vm); //print out time of day as string "14:30:00"

//--------------fileIO natives----------------//

Value readFileNative(int argCount, Value* args, struct Vm* vm); //get all content from file
//Value readLineFileNative(int argCount, Value* args, struct Vm* vm);  //read only one line in the file //TODO: maybe add these later with some state
//Value readWordFileNative(int argCount, Value* args, struct Vm* vm);  //read only one word in the file //TODO: maybe add these later with some state
Value writeFileNative(int argCount, Value* args, struct Vm* vm);     //create a new file and then write to it
Value appendFileNative(int argCount, Value* args, struct Vm* vm);    //write onto the file, append to it
Value fileExistsNative(int argCount, Value* args, struct Vm* vm);    //bool to see if file exists at path
Value deleteFileNative(int argCount, Value* args, struct Vm* vm);    //delete file at path

//--------------Type natives----------------//  //TODO: update this if I ever add longs and shorts
Value intToStrNative(int argCount, Value* args, struct Vm* vm);
Value intToDoubleNative(int argCount, Value* args, struct Vm* vm);
Value intToFloatNative(int argCount, Value* args, struct Vm* vm);

Value doubleToStrNative(int argCount, Value* args, struct Vm* vm);
Value doubleToIntNative(int argCount, Value* args, struct Vm* vm);
Value doubleToFloatNative(int argCount, Value* args, struct Vm* vm);

Value floatToStrNative(int argCount, Value* args, struct Vm* vm);
Value floatToIntNative(int argCount, Value* args, struct Vm* vm);
Value floatToDoubleNative(int argCount, Value* args, struct Vm* vm);

Value strToFloatNative(int argCount, Value* args, struct Vm* vm);
Value strToIntNative(int argCount, Value* args, struct Vm* vm);
Value strToDoubleNative(int argCount, Value* args, struct Vm* vm);
Value strToBoolNative(int argCount, Value* args, struct Vm* vm);

Value boolToStrNative(int argCount, Value* args, struct Vm* vm);

//--------------hashMap natives----------------//
//TODO: do later
//--------------dynamic array natives----------------//
//TODO: do later

//--------------utils natives----------------//
Value lengthNative(int argCount, Value* args, struct Vm* vm);
Value assertNative(int argCount, Value* args, struct Vm* vm);

#endif //OLI_NAT_NATIVES_H
