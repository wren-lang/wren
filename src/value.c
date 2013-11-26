#include "value.h"
#include "vm.h"

int valuesEqual(Value a, Value b)
{
#ifdef NAN_TAGGING
  return a.bits == b.bits;
#else
  if (a.type != b.type) return 0;
  if (a.type == VAL_NUM) return a.num == b.num;
  return a.obj == b.obj;
#endif
}


// Returns the class of [object].
ObjClass* wrenGetClass(WrenVM* vm, Value value)
{
#ifdef NAN_TAGGING
  if (IS_NUM(value)) return vm->numClass;
  if (IS_OBJ(value))
  {
    Obj* obj = AS_OBJ(value);
    switch (obj->type)
    {
      case OBJ_CLASS: return AS_CLASS(value)->metaclass;
      case OBJ_FN: return vm->fnClass;
      case OBJ_INSTANCE: return AS_INSTANCE(value)->classObj;
      case OBJ_LIST: return vm->listClass;
      case OBJ_STRING: return vm->stringClass;
    }
  }

  switch (GET_TAG(value))
  {
    case TAG_FALSE: return vm->boolClass;
    case TAG_NAN: return vm->numClass;
    case TAG_NULL: return vm->nullClass;
    case TAG_TRUE: return vm->boolClass;
  }

  return NULL;
#else
  switch (value.type)
  {
    case VAL_FALSE: return vm->boolClass;
    case VAL_NULL: return vm->nullClass;
    case VAL_NUM: return vm->numClass;
    case VAL_TRUE: return vm->boolClass;
    case VAL_OBJ:
    {
      switch (value.obj->type)
      {
        case OBJ_CLASS: return AS_CLASS(value)->metaclass;
        case OBJ_FN: return vm->fnClass;
        case OBJ_LIST: return vm->listClass;
        case OBJ_STRING: return vm->stringClass;
        case OBJ_INSTANCE: return AS_INSTANCE(value)->classObj;
      }
    }
  }
#endif
}

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
