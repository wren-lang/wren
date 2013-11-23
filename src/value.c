#include "value.h"

int valueIsFn(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_FN;
}

int valueIsInstance(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_INSTANCE;
}

int valueIsString(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_STRING;
}

#ifdef NAN_TAGGING

int valueIsBool(Value value)
{
  return value.bits == TRUE_VAL.bits || value.bits == FALSE_VAL.bits;
}

Value objectToValue(Obj* obj)
{
  return (Value)(SIGN_BIT | QNAN | (uint64_t)(obj));
}

#else

int valueIsBool(Value value)
{
  return value.type == VAL_FALSE || value.type == VAL_TRUE;
}

Value objectToValue(Obj* obj)
{
  Value value;
  value.type = VAL_OBJ;
  value.obj = obj;
  return value;
}

#endif
