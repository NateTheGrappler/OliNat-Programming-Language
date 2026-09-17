#include "common.h"

ValueType toArrayType(ValueType base)
{
    switch (base)
    {
        case VALUE_INT:    return VALUE_INT_ARRAY;
        case VALUE_FLOAT:  return VALUE_FLOAT_ARRAY;
        case VALUE_DOUBLE: return VALUE_DOUBLE_ARRAY;
        case VALUE_STRING: return VALUE_STRING_ARRAY;
        case VALUE_BOOL:   return VALUE_BOOL_ARRAY;
        default: return VALUE_ERROR;
    }
}
ValueType toElementType(ValueType arrayType)
{
    switch (arrayType)
    {
        case VALUE_INT_ARRAY:    return VALUE_INT;
        case VALUE_FLOAT_ARRAY:  return VALUE_FLOAT;
        case VALUE_DOUBLE_ARRAY: return VALUE_DOUBLE;
        case VALUE_STRING_ARRAY: return VALUE_STRING;
        case VALUE_BOOL_ARRAY:   return VALUE_BOOL;
        default: return VALUE_ERROR;
    }
}

