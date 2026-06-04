//
// Created by natang on 6/3/26.
//

#ifndef OLI_NAT_OBJECT_H
#define OLI_NAT_OBJECT_H


typedef enum
{
    OBJ_STRING,
} ObjType;

//set up object class to be stored inside of value struct
typedef struct Obj
{
    ObjType type;
    struct Obj* next; //for storing in vm
} Obj;


//string representation for Oli-Nat
typedef struct ObjString
{
    Obj obj;
    int length;
    char* chars;
};



#endif //OLI_NAT_OBJECT_H
