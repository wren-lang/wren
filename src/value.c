#include <stdio.h>

#include "value.h"
#include "vm.h"

int wrenValuesEqual(Value a, Value b)
{
#ifdef NAN_TAGGING
  return a.bits == b.bits;
#else
  if (a.type != b.type) return 0;
  if (a.type == VAL_NUM) return a.num == b.num;
  return a.obj == b.obj;
#endif
}

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

static void printList(ObjList* list)
{
  printf("[");
  for (int i = 0; i < list->count; i++)
  {
    if (i > 0) printf(", ");
    wrenPrintValue(list->elements[i]);
  }
  printf("]");
}

void wrenPrintValue(Value value)
{
  // TODO(bob): Unify these.
#ifdef NAN_TAGGING
  if (IS_NUM(value))
  {
    printf("%.14g", AS_NUM(value));
  }
  else if (IS_OBJ(value))
  {
    Obj* obj = AS_OBJ(value);
    switch (obj->type)
    {
      case OBJ_CLASS: printf("[class %p]", obj); break;
      case OBJ_FN: printf("[fn %p]", obj); break;
      case OBJ_INSTANCE: printf("[instance %p]", obj); break;
      case OBJ_LIST: printList((ObjList*)obj); break;
      case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
    }
  }
  else
  {
    switch (GET_TAG(value))
    {
      case TAG_FALSE: printf("false"); break;
      case TAG_NAN: printf("NaN"); break;
      case TAG_NULL: printf("null"); break;
      case TAG_TRUE: printf("true"); break;
    }
  }
#else
  switch (value.type)
  {
    case VAL_FALSE: printf("false"); break;
    case VAL_NULL: printf("null"); break;
    case VAL_NUM: printf("%.14g", AS_NUM(value)); break;
    case VAL_TRUE: printf("true"); break;
    case VAL_OBJ:
      switch (value.obj->type)
    {
      case OBJ_CLASS: printf("[class %p]", value.obj); break;
      case OBJ_FN: printf("[fn %p]", value.obj); break;
      case OBJ_INSTANCE: printf("[instance %p]", value.obj); break;
      case OBJ_LIST: printList((ObjList*)value.obj); break;
      case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
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
