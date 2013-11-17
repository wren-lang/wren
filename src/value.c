#include "value.h"

int valueIsFn(Value value)
{
  return value.type == VAL_OBJ && value.obj->type == OBJ_FN;
}

int valueIsString(Value value)
{
  return value.type == VAL_OBJ && value.obj->type == OBJ_STRING;
}

Value objectToValue(Obj* obj)
{
  Value value;
  value.type = VAL_OBJ;
  value.obj = obj;
  return value;
}
