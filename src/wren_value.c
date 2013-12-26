#include <stdio.h>
#include <string.h>

#include "wren_value.h"
#include "wren_vm.h"

static void* allocate(WrenVM* vm, size_t size)
{
  return wrenReallocate(vm, NULL, 0, size);
}

static void initObj(WrenVM* vm, Obj* obj, ObjType type)
{
  obj->type = type;
  obj->flags = 0;
  obj->next = vm->first;
  vm->first = obj;
}

ObjClass* wrenNewSingleClass(WrenVM* vm, int numFields)
{
  ObjClass* classObj = allocate(vm, sizeof(ObjClass));
  initObj(vm, &classObj->obj, OBJ_CLASS);
  classObj->metaclass = NULL;
  classObj->superclass = NULL;
  classObj->numFields = numFields;

  // Clear out the method table.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    classObj->methods[i].type = METHOD_NONE;
  }

  return classObj;
}

void wrenBindSuperclass(ObjClass* subclass, ObjClass* superclass)
{
  ASSERT(superclass != NULL, "Must have superclass.");

  subclass->superclass = superclass;

  // Include the superclass in the total number of fields.
  subclass->numFields += superclass->numFields;

  // Inherit methods from its superclass.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    subclass->methods[i] = superclass->methods[i];
  }
}

ObjClass* wrenNewClass(WrenVM* vm, ObjClass* superclass, int numFields)
{
  // Create the metaclass.
  // TODO: Handle static fields.
  ObjClass* metaclass = wrenNewSingleClass(vm, 0);
  metaclass->metaclass = vm->classClass;

  // Make sure the metaclass isn't collected when we allocate the class.
  PinnedObj pinned;
  pinObj(vm, (Obj*)metaclass, &pinned);

  // The metaclass inheritance chain mirrors the class's inheritance chain
  // except that when the latter bottoms out at "Object", the metaclass one
  // bottoms out at "Class".
  if (superclass == vm->objectClass)
  {
    wrenBindSuperclass(metaclass, vm->classClass);
  }
  else
  {
    wrenBindSuperclass(metaclass, superclass->metaclass);
  }

  ObjClass* classObj = wrenNewSingleClass(vm, numFields);
  classObj->metaclass = metaclass;
  wrenBindSuperclass(classObj, superclass);

  unpinObj(vm);

  return classObj;
}

ObjClosure* wrenNewClosure(WrenVM* vm, ObjFn* fn)
{
  ObjClosure* closure = allocate(vm,
      sizeof(ObjClosure) + sizeof(Upvalue*) * fn->numUpvalues);
  initObj(vm, &closure->obj, OBJ_CLOSURE);

  closure->fn = fn;

  // Clear the upvalue array. We need to do this in case a GC is triggered
  // after the closure is created but before the upvalue array is populated.
  for (int i = 0; i < fn->numUpvalues; i++) closure->upvalues[i] = NULL;

  return closure;
}

ObjFiber* wrenNewFiber(WrenVM* vm)
{
  ObjFiber* fiber = allocate(vm, sizeof(ObjFiber));
  initObj(vm, &fiber->obj, OBJ_FIBER);

  fiber->stackSize = 0;
  fiber->numFrames = 0;
  fiber->openUpvalues = NULL;

  return fiber;
}

ObjFn* wrenNewFunction(WrenVM* vm)
{
  // Allocate these before the function in case they trigger a GC which would
  // free the function.
  // TODO: Hack! make variable sized.
  unsigned char* bytecode = allocate(vm, sizeof(Code) * 1024);
  Value* constants = allocate(vm, sizeof(Value) * 256);

  ObjFn* fn = allocate(vm, sizeof(ObjFn));
  initObj(vm, &fn->obj, OBJ_FN);

  fn->numConstants = 0;
  fn->numUpvalues = 0;
  fn->bytecode = bytecode;
  fn->constants = constants;

  return fn;
}

Value wrenNewInstance(WrenVM* vm, ObjClass* classObj)
{
  ObjInstance* instance = allocate(vm,
                                   sizeof(ObjInstance) + classObj->numFields * sizeof(Value));
  initObj(vm, &instance->obj, OBJ_INSTANCE);
  instance->classObj = classObj;

  // Initialize fields to null.
  for (int i = 0; i < classObj->numFields; i++)
  {
    instance->fields[i] = NULL_VAL;
  }

  return OBJ_VAL(instance);
}

ObjList* wrenNewList(WrenVM* vm, int numElements)
{
  // Allocate this before the list object in case it triggers a GC which would
  // free the list.
  Value* elements = NULL;
  if (numElements > 0)
  {
    elements = allocate(vm, sizeof(Value) * numElements);
  }

  ObjList* list = allocate(vm, sizeof(ObjList));
  initObj(vm, &list->obj, OBJ_LIST);
  list->capacity = numElements;
  list->count = numElements;
  list->elements = elements;
  return list;
}

Value wrenNewString(WrenVM* vm, const char* text, size_t length)
{
  // Allocate before the string object in case this triggers a GC which would
  // free the string object.
  char* heapText = allocate(vm, length + 1);

  ObjString* string = allocate(vm, sizeof(ObjString));
  initObj(vm, &string->obj, OBJ_STRING);
  string->value = heapText;

  // Copy the string (if given one).
  if (text != NULL)
  {
    strncpy(heapText, text, length);
    heapText[length] = '\0';
  }

  return OBJ_VAL(string);
}

Upvalue* wrenNewUpvalue(WrenVM* vm, Value* value)
{
  Upvalue* upvalue = allocate(vm, sizeof(Upvalue));
  initObj(vm, &upvalue->obj, OBJ_UPVALUE);

  upvalue->value = value;
  upvalue->next = NULL;
  return upvalue;
}

static ObjClass* getObjectClass(WrenVM* vm, Obj* obj)
{
  switch (obj->type)
  {
    case OBJ_CLASS:
    {
      ObjClass* classObj = (ObjClass*)obj;
      return classObj->metaclass;
    }
    case OBJ_CLOSURE: return vm->fnClass;
    case OBJ_FIBER: return vm->fiberClass;
    case OBJ_FN: return vm->fnClass;
    case OBJ_INSTANCE: return ((ObjInstance*)obj)->classObj;
    case OBJ_LIST: return vm->listClass;
    case OBJ_STRING: return vm->stringClass;
    case OBJ_UPVALUE:
      ASSERT(0, "Upvalues should not be used as first-class objects.");
      return NULL;
    default:
      ASSERT(0, "Unreachable.");
      return NULL;
  }
}

ObjClass* wrenGetClass(WrenVM* vm, Value value)
{
  #if WREN_NAN_TAGGING
  if (IS_NUM(value)) return vm->numClass;
  if (IS_OBJ(value)) return getObjectClass(vm, AS_OBJ(value));

  switch (GET_TAG(value))
  {
    case TAG_FALSE: return vm->boolClass;
    case TAG_NAN: return vm->numClass;
    case TAG_NULL: return vm->nullClass;
    case TAG_TRUE: return vm->boolClass;
  }
  #else
  switch (value.type)
  {
    case VAL_FALSE: return vm->boolClass;
    case VAL_NULL: return vm->nullClass;
    case VAL_NUM: return vm->numClass;
    case VAL_TRUE: return vm->boolClass;
    case VAL_OBJ: return getObjectClass(vm, value.obj);
  }
  #endif

  return NULL; // Unreachable.
}

bool wrenValuesEqual(Value a, Value b)
{
  #if WREN_NAN_TAGGING
  // Value types have unique bit representations and we compare object types
  // by identity (i.e. pointer), so all we need to do is compare the bits.
  return a.bits == b.bits;
  #else
  if (a.type != b.type) return false;
  if (a.type == VAL_NUM) return a.num == b.num;
  return a.obj == b.obj;
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

static void printObject(Obj* obj)
{
  switch (obj->type)
  {
    case OBJ_CLASS: printf("[class %p]", obj); break;
    case OBJ_CLOSURE: printf("[closure %p]", obj); break;
    case OBJ_FIBER: printf("[fiber %p]", obj); break;
    case OBJ_FN: printf("[fn %p]", obj); break;
    case OBJ_INSTANCE: printf("[instance %p]", obj); break;
    case OBJ_LIST: printList((ObjList*)obj); break;
    case OBJ_STRING: printf("%s", ((ObjString*)obj)->value); break;
    case OBJ_UPVALUE: printf("[upvalue %p]", obj); break;
  }
}

void wrenPrintValue(Value value)
{
  #if WREN_NAN_TAGGING
  if (IS_NUM(value))
  {
    printf("%.14g", AS_NUM(value));
  }
  else if (IS_OBJ(value))
  {
    printObject(AS_OBJ(value));
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
    {
      printObject(AS_OBJ(value));
    }
  }
  #endif
}

bool wrenIsBool(Value value)
{
  #if WREN_NAN_TAGGING
  return value.bits == TRUE_VAL.bits || value.bits == FALSE_VAL.bits;
  #else
  return value.type == VAL_FALSE || value.type == VAL_TRUE;
  #endif
}

bool wrenIsClosure(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_CLOSURE;
}

bool wrenIsFiber(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_FIBER;
}

bool wrenIsFn(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_FN;
}

bool wrenIsInstance(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_INSTANCE;
}

bool wrenIsString(Value value)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == OBJ_STRING;
}

extern Value wrenObjectToValue(Obj* obj);
