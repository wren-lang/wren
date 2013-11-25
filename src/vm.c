#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "primitives.h"
#include "vm.h"

VM* newVM()
{
  // TODO(bob): Get rid of explicit malloc() here.
  VM* vm = malloc(sizeof(VM));
  initSymbolTable(&vm->methods);
  initSymbolTable(&vm->globalSymbols);

  // TODO(bob): Get rid of explicit malloc() here.
  vm->fiber = malloc(sizeof(Fiber));
  vm->fiber->stackSize = 0;
  vm->fiber->numFrames = 0;
  vm->totalAllocated = 0;

  // TODO(bob): Make this configurable.
  vm->nextGC = 1024 * 1024 * 10;

  // Clear out the global variables. This ensures they are NULL before being
  // initialized in case we do a garbage collection before one gets initialized.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    vm->globals[i] = NULL_VAL;
  }

  loadCore(vm);

  return vm;
}

void freeVM(VM* vm)
{
  clearSymbolTable(&vm->methods);
  clearSymbolTable(&vm->globalSymbols);
  free(vm);
}

static void collectGarbage(VM* vm);

// A generic allocation that handles all memory changes, like so:
//
// - To allocate new memory, [memory] is NULL and [oldSize] is zero.
//
// - To attempt to grow an existing allocation, [memory] is the memory,
//   [oldSize] is its previous size, and [newSize] is the desired size.
//   It returns [memory] if it was able to grow it in place, or a new pointer
//   if it had to move it.
//
// - To shrink memory, [memory], [oldSize], and [newSize] are the same as above
//   but it will always return [memory]. If [newSize] is zero, the memory will
//   be freed and `NULL` will be returned.
//
// - To free memory, [newSize] will be zero.
void* reallocate(VM* vm, void* memory, size_t oldSize, size_t newSize)
{
  ASSERT(memory == NULL || oldSize > 0, "Cannot take unsized previous memory.");

  vm->totalAllocated += newSize - oldSize;

#ifdef DEBUG_GC_STRESS
  if (newSize > oldSize)
  {
    collectGarbage(vm);
  }
#else
  if (vm->totalAllocated > vm->nextGC)
  {
#ifdef TRACE_MEMORY
    size_t before = vm->totalAllocated;
#endif
    collectGarbage(vm);
    vm->nextGC = vm->totalAllocated * 3 / 2;

#ifdef TRACE_MEMORY
    printf("GC %ld before, %ld after (%ld collected), next at %ld\n",
           before, vm->totalAllocated, before - vm->totalAllocated, vm->nextGC);
#endif
  }
#endif

  if (newSize == 0)
  {
    ASSERT(memory != NULL, "Must have pointer to free.");
    free(memory);
    return NULL;
  }

  // TODO(bob): Let external code provide allocator.
  return realloc(memory, newSize);
}

static void* allocate(VM* vm, size_t size)
{
  return reallocate(vm, NULL, 0, size);
}

static void* deallocate(VM* vm, void* memory, size_t oldSize)
{
  return reallocate(vm, memory, oldSize, 0);
}

static void markValue(Value value);

static void markFn(ObjFn* fn)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (fn->obj.flags & FLAG_MARKED) return;
  fn->obj.flags |= FLAG_MARKED;

  // Mark the constants.
  for (int i = 0; i < fn->numConstants; i++)
  {
    markValue(fn->constants[i]);
  }
}

static void markClass(ObjClass* classObj)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (classObj->obj.flags & FLAG_MARKED) return;
  classObj->obj.flags |= FLAG_MARKED;

  // The metaclass.
  if (classObj->metaclass != NULL) markClass(classObj->metaclass);

  // The superclass.
  if (classObj->superclass != NULL) markClass(classObj->superclass);

  // Method function objects.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    if (classObj->methods[i].type == METHOD_BLOCK)
    {
      markFn(classObj->methods[i].fn);
    }
  }
}

static void markInstance(ObjInstance* instance)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (instance->obj.flags & FLAG_MARKED) return;
  instance->obj.flags |= FLAG_MARKED;

  markClass(instance->classObj);

  // Mark the fields.
  for (int i = 0; i < instance->classObj->numFields; i++)
  {
    markValue(instance->fields[i]);
  }
}

static void markList(ObjList* list)
{
  Value* elements = list->elements;
  for (int i = 0; i < list->count; i++)
  {
    markValue(elements[i]);
  }
}

static void markObj(Obj* obj)
{
#ifdef TRACE_MEMORY
  static int indent = 0;
  indent++;
  for (int i = 0; i < indent; i++) printf("  ");
  printf("mark ");
  printValue(OBJ_VAL(obj));
  printf("\n");
#endif

  // Traverse the object's fields.
  switch (obj->type)
  {
    case OBJ_CLASS: markClass((ObjClass*)obj); break;
    case OBJ_FN: markFn((ObjFn*)obj); break;
    case OBJ_INSTANCE: markInstance((ObjInstance*)obj); break;
    case OBJ_LIST: markList((ObjList*)obj); break;
    case OBJ_STRING:
      // Just mark the string itself.
      obj->flags |= FLAG_MARKED;
      break;
  }

#ifdef TRACE_MEMORY
  indent--;
#endif
}

void markValue(Value value)
{
  if (!IS_OBJ(value)) return;
  markObj(AS_OBJ(value));
}

void freeObj(VM* vm, Obj* obj)
{
#ifdef TRACE_MEMORY
  printf("free ");
  printValue(OBJ_VAL(obj));
#endif

  // Free any additional heap data allocated by the object.
  size_t size;

  switch (obj->type)
  {
    case OBJ_CLASS:
      size = sizeof(ObjClass);
      break;

    case OBJ_FN:
    {
      // TODO(bob): Don't hardcode array sizes.
      size = sizeof(ObjFn);
      ObjFn* fn = (ObjFn*)obj;
      deallocate(vm, fn->bytecode, sizeof(Code) * 1024);
      deallocate(vm, fn->constants, sizeof(Value) * 256);
      break;
    }

    case OBJ_INSTANCE:
    {
      size = sizeof(ObjInstance);

      // Include the size of the field array.
      ObjInstance* instance = (ObjInstance*)obj;
      size += sizeof(Value) * instance->classObj->numFields;
      break;
    }

    case OBJ_LIST:
    {
      size = sizeof(ObjList);
      ObjList* list = (ObjList*)obj;
      if (list->elements != NULL)
      {
        deallocate(vm, list->elements, sizeof(Value) * list->count);
      }
      break;
    }

    case OBJ_STRING:
    {
      size = sizeof(ObjString);
      ObjString* string = (ObjString*)obj;
      // TODO(bob): O(n) calculation here is lame!
      deallocate(vm, string->value, strlen(string->value));
      break;
    }
  }

  deallocate(vm, obj, size);
}

void collectGarbage(VM* vm)
{
  // Mark all reachable objects.
#ifdef TRACE_MEMORY
  printf("-- gc --\n");
#endif

  // Global variables.
  for (int i = 0; i < vm->globalSymbols.count; i++)
  {
    // Check for NULL to handle globals that have been defined (at compile time)
    // but not yet initialized.
    if (!IS_NULL(vm->globals[i])) markValue(vm->globals[i]);
  }

  // Pinned objects.
  for (int j = 0; j < vm->numPinned; j++)
  {
    markObj(vm->pinned[j]);
  }

  // Stack functions.
  for (int k = 0; k < vm->fiber->numFrames; k++)
  {
    markFn(vm->fiber->frames[k].fn);
  }

  // Stack variables.
  for (int l = 0; l < vm->fiber->stackSize; l++)
  {
    markValue(vm->fiber->stack[l]);
  }

  // Collect any unmarked objects.
  Obj** obj = &vm->first;
  while (*obj != NULL)
  {
    if (!((*obj)->flags & FLAG_MARKED))
    {
      // This object wasn't reached, so remove it from the list and free it.
      Obj* unreached = *obj;
      *obj = unreached->next;
      freeObj(vm, unreached);
    }
    else
    {
      // This object was reached, so unmark it (for the next GC) and move on to
      // the next.
      (*obj)->flags &= ~FLAG_MARKED;
      obj = &(*obj)->next;
    }
  }
}

void initObj(VM* vm, Obj* obj, ObjType type)
{
  obj->type = type;
  obj->flags = 0;
  obj->next = vm->first;
  vm->first = obj;
}

static ObjClass* newSingleClass(VM* vm, ObjClass* metaclass,
                                ObjClass* superclass, int numFields)
{
  ObjClass* obj = allocate(vm, sizeof(ObjClass));
  initObj(vm, &obj->obj, OBJ_CLASS);
  obj->metaclass = metaclass;
  obj->superclass = superclass;
  obj->numFields = numFields;

  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    obj->methods[i].type = METHOD_NONE;
  }

  return obj;
}

ObjClass* newClass(VM* vm, ObjClass* superclass, int numFields)
{
  // Make the metaclass.
  // TODO(bob): What is the metaclass's metaclass and superclass?
  // TODO(bob): Handle static fields.
  ObjClass* metaclass = newSingleClass(vm, NULL, NULL, 0);

  // Make sure it isn't collected when we allocate the metaclass.
  pinObj(vm, (Obj*)metaclass);

  ObjClass* classObj = newSingleClass(vm, metaclass, superclass, numFields);
  classObj->numFields = numFields;

  unpinObj(vm, (Obj*)metaclass);

  // Inherit methods from its superclass (unless it's Object, which has none).
  // TODO(bob): If we want BETA-style inheritance, we'll need to do this after
  // the subclass has defined its methods.
  if (superclass != NULL)
  {
    for (int i = 0; i < MAX_SYMBOLS; i++)
    {
      classObj->methods[i] = superclass->methods[i];
    }
  }

  return classObj;
}

ObjFn* newFunction(VM* vm)
{
  // Allocate these before the function in case they trigger a GC which would
  // free the function.
  // TODO(bob): Hack! make variable sized.
  unsigned char* bytecode = allocate(vm, sizeof(Code) * 1024);
  Value* constants = allocate(vm, sizeof(Value) * 256);

  ObjFn* fn = allocate(vm, sizeof(ObjFn));
  initObj(vm, &fn->obj, OBJ_FN);

  fn->bytecode = bytecode;
  fn->constants = constants;

  return fn;
}

Value newInstance(VM* vm, ObjClass* classObj)
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

ObjList* newList(VM* vm, int numElements)
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
  list->count = numElements;
  list->elements = elements;
  return list;
}

Value newString(VM* vm, const char* text, size_t length)
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

void initSymbolTable(SymbolTable* symbols)
{
  symbols->count = 0;
}

void clearSymbolTable(SymbolTable* symbols)
{
  for (int i = 0; i < symbols->count; i++)
  {
    free(symbols->names[i]);
  }
}

void truncateSymbolTable(SymbolTable* symbols, int count)
{
  ASSERT(count <= symbols->count, "Cannot truncate to larger size.");
  for (int i = count; i < symbols->count; i++)
  {
    free(symbols->names[i]);
  }
  symbols->count = count;
}

int addSymbolUnchecked(SymbolTable* symbols, const char* name, size_t length)
{
  // TODO(bob): Get rid of explicit malloc here.
  symbols->names[symbols->count] = malloc(length + 1);
  strncpy(symbols->names[symbols->count], name, length);
  symbols->names[symbols->count][length] = '\0';

  return symbols->count++;
}

int addSymbol(SymbolTable* symbols, const char* name, size_t length)
{
  // If already present, return an error.
  if (findSymbol(symbols, name, length) != -1) return -1;

  return addSymbolUnchecked(symbols, name, length);
}

int ensureSymbol(SymbolTable* symbols, const char* name, size_t length)
{
  // See if the symbol is already defined.
  int existing = findSymbol(symbols, name, length);
  if (existing != -1) return existing;

  // New symbol, so add it.
  return addSymbolUnchecked(symbols, name, length);
}

int findSymbol(SymbolTable* symbols, const char* name, size_t length)
{
  // See if the symbol is already defined.
  // TODO(bob): O(n). Do something better.
  for (int i = 0; i < symbols->count; i++)
  {
    if (strlen(symbols->names[i]) == length &&
        strncmp(symbols->names[i], name, length) == 0) return i;
  }

  return -1;
}

const char* getSymbolName(SymbolTable* symbols, int symbol)
{
  return symbols->names[symbol];
}

Value findGlobal(VM* vm, const char* name)
{
  int symbol = findSymbol(&vm->globalSymbols, name, strlen(name));
  // TODO(bob): Handle failure.
  return vm->globals[symbol];
}

int dumpInstruction(VM* vm, ObjFn* fn, int i)
{
  int start = i;
  unsigned char* bytecode = fn->bytecode;
  unsigned char code = bytecode[i++];

  printf("%04d  ", i);

  switch (code)
  {
    case CODE_CONSTANT:
    {
      int constant = bytecode[i++];
      printf("CONSTANT ");
      printValue(fn->constants[constant]);
      printf("\n");
      printf("%04d   | constant %d\n", i, constant);
      break;
    }

    case CODE_NULL:
      printf("NULL\n");
      break;

    case CODE_FALSE:
      printf("FALSE\n");
      break;

    case CODE_TRUE:
      printf("TRUE\n");
      break;

    case CODE_CLASS:
    {
      int numFields = bytecode[i++];
      printf("CLASS\n");
      printf("%04d   | num fields %d\n", i, numFields);
      break;
    }

    case CODE_SUBCLASS:
    {
      int numFields = bytecode[i++];
      printf("SUBCLASS\n");
      printf("%04d   | num fields %d\n", i, numFields);
      break;
    }

    case CODE_METHOD_INSTANCE:
    {
      int symbol = bytecode[i++];
      int constant = bytecode[i++];
      printf("METHOD_INSTANCE \"%s\"\n", getSymbolName(&vm->methods, symbol));
      printf("%04d   | symbol %d\n", i - 1, symbol);
      printf("%04d   | constant %d\n", i, constant);
      break;
    }

    case CODE_METHOD_STATIC:
    {
      int symbol = bytecode[i++];
      int constant = bytecode[i++];
      printf("METHOD_STATIC \"%s\"\n", getSymbolName(&vm->methods, symbol));
      printf("%04d   | symbol %d\n", i - 1, symbol);
      printf("%04d   | constant %d\n", i, constant);
      break;
    }

    case CODE_METHOD_CTOR:
    {
      int symbol = bytecode[i++];
      int constant = bytecode[i++];
      printf("METHOD_CTOR \"%s\"\n", getSymbolName(&vm->methods, symbol));
      printf("%04d   | symbol %d\n", i - 1, symbol);
      printf("%04d   | constant %d\n", i, constant);
      break;
    }

    case CODE_LIST:
    {
      int count = bytecode[i++];
      printf("LIST\n");
      printf("%04d   | count %d\n", i, count);
      break;
    }

    case CODE_LOAD_LOCAL:
    {
      int local = bytecode[i++];
      printf("LOAD_LOCAL %d\n", local);
      printf("%04d   | local %d\n", i, local);
      break;
    }

    case CODE_STORE_LOCAL:
    {
      int local = bytecode[i++];
      printf("STORE_LOCAL %d\n", local);
      printf("%04d   | local %d\n", i, local);
      break;
    }

    case CODE_LOAD_GLOBAL:
    {
      int global = bytecode[i++];
      printf("LOAD_GLOBAL \"%s\"\n",
             getSymbolName(&vm->globalSymbols, global));
      printf("%04d   | global %d\n", i, global);
      break;
    }

    case CODE_STORE_GLOBAL:
    {
      int global = bytecode[i++];
      printf("STORE_GLOBAL \"%s\"\n",
             getSymbolName(&vm->globalSymbols, global));
      printf("%04d   | global %d\n", i, global);
      break;
    }

    case CODE_LOAD_FIELD:
    {
      int field = bytecode[i++];
      printf("LOAD_FIELD %d\n", field);
      printf("%04d   | field %d\n", i, field);
      break;
    }

    case CODE_STORE_FIELD:
    {
      int field = bytecode[i++];
      printf("STORE_FIELD %d\n", field);
      printf("%04d   | field %d\n", i, field);
      break;
    }

    case CODE_DUP:
      printf("DUP\n");
      break;

    case CODE_POP:
      printf("POP\n");
      break;

    case CODE_CALL_0:
    case CODE_CALL_1:
    case CODE_CALL_2:
    case CODE_CALL_3:
    case CODE_CALL_4:
    case CODE_CALL_5:
    case CODE_CALL_6:
    case CODE_CALL_7:
    case CODE_CALL_8:
    case CODE_CALL_9:
    case CODE_CALL_10:
    {
      // Add one for the implicit receiver argument.
      int numArgs = bytecode[i - 1] - CODE_CALL_0;
      int symbol = bytecode[i++];
      printf("CALL_%d \"%s\"\n", numArgs,
             getSymbolName(&vm->methods, symbol));
      printf("%04d   | symbol %d\n", i, symbol);
      break;
    }

    case CODE_JUMP:
    {
      int offset = bytecode[i++];
      printf("JUMP %d\n", offset);
      printf("%04d   | offset %d\n", i, offset);
      break;
    }

    case CODE_LOOP:
    {
      int offset = bytecode[i++];
      printf("LOOP %d\n", offset);
      printf("%04d   | offset -%d\n", i, offset);
      break;
    }

    case CODE_JUMP_IF:
    {
      int offset = bytecode[i++];
      printf("JUMP_IF %d\n", offset);
      printf("%04d   | offset %d\n", i, offset);
      break;
    }

    case CODE_AND:
    {
      int offset = bytecode[i++];
      printf("AND %d\n", offset);
      printf("%04d   | offset %d\n", i, offset);
      break;
    }

    case CODE_OR:
    {
      int offset = bytecode[i++];
      printf("OR %d\n", offset);
      printf("%04d   | offset %d\n", i, offset);
      break;
    }

    case CODE_IS:
      printf("CODE_IS\n");
      break;

    case CODE_END:
      printf("CODE_END\n");
      break;

    default:
      printf("UKNOWN! [%d]\n", bytecode[i - 1]);
      break;
  }

  // Return how many bytes this instruction takes, or -1 if it's an END.
  if (code == CODE_END) return -1;
  return i - start;
}

// TODO(bob): For debugging. Move to separate module.
void dumpCode(VM* vm, ObjFn* fn)
{
  int i = 0;
  for (;;)
  {
    int offset = dumpInstruction(vm, fn, i);
    if (offset == -1) break;
    i += offset;
  }
}

// Returns the class of [object].
static ObjClass* getClass(VM* vm, Value value)
{  // TODO(bob): Unify these.
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

void dumpStack(Fiber* fiber)
{
  printf(":: ");
  for (int i = 0; i < fiber->stackSize; i++)
  {
    printValue(fiber->stack[i]);
    printf(" | ");
  }
  printf("\n");
}

Value interpret(VM* vm, ObjFn* fn)
{
  Fiber* fiber = vm->fiber;
  callFunction(fiber, fn, 0);

  // These macros are designed to only be invoked within this function.
  // TODO(bob): Check for stack overflow.
  #define PUSH(value) (fiber->stack[fiber->stackSize++] = value)
  #define POP()       (fiber->stack[--fiber->stackSize])
  #define PEEK()      (fiber->stack[fiber->stackSize - 1])
  #define READ_ARG()  (frame->ip++, bytecode[ip++])

  // Hoist these into local variables. They are accessed frequently in the loop
  // but change less frequently. Keeping them in locals and updating them when
  // a call frame has been pushed or pop gives a large speed boost.
  CallFrame* frame = &fiber->frames[fiber->numFrames - 1];
  int ip = frame->ip;
  unsigned char* bytecode = frame->fn->bytecode;

  // Use this before a CallFrame is pushed to store the local variables back
  // into the current one.
  #define STORE_FRAME() frame->ip = ip

  // Use this after a CallFrame has been pushed or popped to refresh the local
  // variables.
  #define LOAD_FRAME() \
      frame = &fiber->frames[fiber->numFrames - 1]; \
      ip = frame->ip; \
      bytecode = frame->fn->bytecode \

  #ifdef COMPUTED_GOTOS

  static void* dispatchTable[] = {
    &&code_CONSTANT,
    &&code_NULL,
    &&code_FALSE,
    &&code_TRUE,
    &&code_CLASS,
    &&code_SUBCLASS,
    &&code_METHOD_INSTANCE,
    &&code_METHOD_STATIC,
    &&code_METHOD_CTOR,
    &&code_LIST,
    &&code_LOAD_LOCAL,
    &&code_STORE_LOCAL,
    &&code_LOAD_GLOBAL,
    &&code_STORE_GLOBAL,
    &&code_LOAD_FIELD,
    &&code_STORE_FIELD,
    &&code_DUP,
    &&code_POP,
    &&code_CALL_0,
    &&code_CALL_1,
    &&code_CALL_2,
    &&code_CALL_3,
    &&code_CALL_4,
    &&code_CALL_5,
    &&code_CALL_6,
    &&code_CALL_7,
    &&code_CALL_8,
    &&code_CALL_9,
    &&code_CALL_10,
    &&code_JUMP,
    &&code_LOOP,
    &&code_JUMP_IF,
    &&code_AND,
    &&code_OR,
    &&code_IS,
    &&code_END
  };

  #define INTERPRET_LOOP    DISPATCH();
  #define CASE_CODE(name)   code_##name
  #define DISPATCH()        goto *dispatchTable[instruction = bytecode[ip++]]

  #else

  #define INTERPRET_LOOP    for (;;) switch (instruction = bytecode[ip++])
  #define CASE_CODE(name)   case CODE_##name
  #define DISPATCH()        break

  #endif

  Code instruction;
  INTERPRET_LOOP
  {
    CASE_CODE(CONSTANT):
      PUSH(frame->fn->constants[READ_ARG()]);
      DISPATCH();

    CASE_CODE(NULL):  PUSH(NULL_VAL); DISPATCH();
    CASE_CODE(FALSE): PUSH(FALSE_VAL); DISPATCH();
    CASE_CODE(TRUE):  PUSH(TRUE_VAL); DISPATCH();

    CASE_CODE(CLASS):
    CASE_CODE(SUBCLASS):
    {
      int isSubclass = instruction == CODE_SUBCLASS;
      int numFields = READ_ARG();

      ObjClass* superclass;
      if (isSubclass)
      {
        // TODO(bob): Handle the superclass not being a class object!
        superclass = AS_CLASS(POP());
      }
      else
      {
        // Implicit Object superclass.
        superclass = vm->objectClass;
      }

      ObjClass* classObj = newClass(vm, superclass, numFields);

      // Assume the first class being defined is Object.
      if (vm->objectClass == NULL)
      {
        vm->objectClass = classObj;
      }

      // Define a "new" method on the metaclass.
      // TODO(bob): Can this be inherited?
      int newSymbol = ensureSymbol(&vm->methods, "new", strlen("new"));
      classObj->metaclass->methods[newSymbol].type = METHOD_CTOR;
      classObj->metaclass->methods[newSymbol].fn = NULL;

      PUSH(OBJ_VAL(classObj));
      DISPATCH();
    }

    CASE_CODE(LIST):
    {
      int numElements = READ_ARG();
      ObjList* list = newList(vm, numElements);
      for (int i = 0; i < numElements; i++)
      {
        list->elements[i] = fiber->stack[fiber->stackSize - numElements + i];
      }

      // Discard the elements.
      fiber->stackSize -= numElements;

      PUSH(OBJ_VAL(list));
      DISPATCH();
    }

    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    CASE_CODE(METHOD_CTOR):
    {
      int type = instruction;
      int symbol = READ_ARG();
      int constant = READ_ARG();
      ObjClass* classObj = AS_CLASS(PEEK());

      switch (type)
      {
        case CODE_METHOD_INSTANCE:
          classObj->methods[symbol].type = METHOD_BLOCK;
          break;

        case CODE_METHOD_STATIC:
          // Statics are defined on the metaclass.
          classObj = classObj->metaclass;
          classObj->methods[symbol].type = METHOD_BLOCK;
          break;

        case CODE_METHOD_CTOR:
          // Constructors are like statics.
          classObj = classObj->metaclass;
          classObj->methods[symbol].type = METHOD_CTOR;
          break;
      }

      ObjFn* body = AS_FN(frame->fn->constants[constant]);
      classObj->methods[symbol].fn = body;
      DISPATCH();
    }

    CASE_CODE(LOAD_LOCAL):
    {
      int local = READ_ARG();
      PUSH(fiber->stack[frame->stackStart + local]);
      DISPATCH();
    }

    CASE_CODE(STORE_LOCAL):
    {
      int local = READ_ARG();
      fiber->stack[frame->stackStart + local] = PEEK();
      DISPATCH();
    }

    CASE_CODE(LOAD_GLOBAL):
    {
      int global = READ_ARG();
      PUSH(vm->globals[global]);
      DISPATCH();
    }

    CASE_CODE(STORE_GLOBAL):
    {
      int global = READ_ARG();
      vm->globals[global] = PEEK();
      DISPATCH();
    }

    CASE_CODE(LOAD_FIELD):
    {
      int field = READ_ARG();
      // TODO(bob): We'll have to do something better here to handle functions
      // inside methods.
      Value receiver = fiber->stack[frame->stackStart];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(STORE_FIELD):
    {
      int field = READ_ARG();
      // TODO(bob): We'll have to do something better here to handle functions
      // inside methods.
      Value receiver = fiber->stack[frame->stackStart];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(DUP): PUSH(PEEK()); DISPATCH();
    CASE_CODE(POP): POP(); DISPATCH();

    CASE_CODE(CALL_0):
    CASE_CODE(CALL_1):
    CASE_CODE(CALL_2):
    CASE_CODE(CALL_3):
    CASE_CODE(CALL_4):
    CASE_CODE(CALL_5):
    CASE_CODE(CALL_6):
    CASE_CODE(CALL_7):
    CASE_CODE(CALL_8):
    CASE_CODE(CALL_9):
    CASE_CODE(CALL_10):
    {
      // Add one for the implicit receiver argument.
      int numArgs = instruction - CODE_CALL_0 + 1;
      int symbol = READ_ARG();

      Value receiver = fiber->stack[fiber->stackSize - numArgs];
      ObjClass* classObj = getClass(vm, receiver);
      Method* method = &classObj->methods[symbol];
      switch (method->type)
      {
        case METHOD_NONE:
          printf("Receiver ");
          printValue(receiver);
          printf(" does not implement method \"%s\".\n",
                 vm->methods.names[symbol]);
          // TODO(bob): Throw an exception or halt the fiber or something.
          exit(1);
          break;

        case METHOD_PRIMITIVE:
        {
          Value* args = &fiber->stack[fiber->stackSize - numArgs];
          Value result = method->primitive(vm, args);

          fiber->stack[fiber->stackSize - numArgs] = result;

          // Discard the stack slots for the arguments (but leave one for
          // the result).
          fiber->stackSize -= numArgs - 1;
          break;
        }

        case METHOD_FIBER:
        {
          STORE_FRAME();
          Value* args = &fiber->stack[fiber->stackSize - numArgs];
          method->fiberPrimitive(vm, fiber, args);
          LOAD_FRAME();
          break;
        }

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(fiber, method->fn, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_CTOR:
        {
          Value instance = newInstance(vm, AS_CLASS(receiver));

          // Store the new instance in the receiver slot so that it can be
          // "this" in the body of the constructor and returned by it.
          fiber->stack[fiber->stackSize - numArgs] = instance;

          if (method->fn == NULL)
          {
            // Default constructor, so no body to call. Just discard the
            // stack slots for the arguments (but leave one for the instance).
            fiber->stackSize -= numArgs - 1;
          }
          else
          {
            // Invoke the constructor body.
            STORE_FRAME();
            callFunction(fiber, method->fn, numArgs);
            LOAD_FRAME();
          }
          break;
        }
      }
      DISPATCH();
    }

    CASE_CODE(JUMP):
    {
      int offset = READ_ARG();
      ip += offset;
      DISPATCH();
    }

    CASE_CODE(LOOP):
    {
      // The loop body's result is on the top of the stack. Since we are
      // looping and running the body again, discard it.
      POP();

      // Jump back to the top of the loop.
      int offset = READ_ARG();
      ip -= offset;
      DISPATCH();
    }

    CASE_CODE(JUMP_IF):
    {
      int offset = READ_ARG();
      Value condition = POP();

      // False is the only falsey value.
      if (IS_FALSE(condition)) ip += offset;
      DISPATCH();
    }

    CASE_CODE(AND):
    {
      int offset = READ_ARG();
      Value condition = PEEK();

      // False is the only falsey value.
      if (!IS_FALSE(condition))
      {
        // Discard the condition and evaluate the right hand side.
        POP();
      }
      else
      {
        // Short-circuit the right hand side.
        ip += offset;
      }
      DISPATCH();
    }

    CASE_CODE(OR):
    {
      int offset = READ_ARG();
      Value condition = PEEK();

      // False is the only falsey value.
      if (IS_FALSE(condition))
      {
        // Discard the condition and evaluate the right hand side.
        POP();
      }
      else
      {
        // Short-circuit the right hand side.
        ip += offset;
      }
      DISPATCH();
    }

    CASE_CODE(IS):
    {
      Value classObj = POP();
      Value obj = POP();

      // TODO(bob): What if classObj is not a class?
      ObjClass* actual = getClass(vm, obj);
      PUSH(BOOL_VAL(actual == AS_CLASS(classObj)));
      DISPATCH();
    }

    CASE_CODE(END):
    {
      Value result = POP();
      fiber->numFrames--;

      // If we are returning from the top-level block, just return the value.
      if (fiber->numFrames == 0) return result;

      // Store the result of the block in the first slot, which is where the
      // caller expects it.
      fiber->stack[frame->stackStart] = result;

      // Discard the stack slots for the call frame (leaving one slot for the
      // result).
      fiber->stackSize = frame->stackStart + 1;
      LOAD_FRAME();
      DISPATCH();
    }
  }
}

void callFunction(Fiber* fiber, ObjFn* fn, int numArgs)
{
  fiber->frames[fiber->numFrames].fn = fn;
  fiber->frames[fiber->numFrames].ip = 0;
  fiber->frames[fiber->numFrames].stackStart = fiber->stackSize - numArgs;

  // TODO(bob): Check for stack overflow.
  fiber->numFrames++;
}

void printValue(Value value)
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
      case OBJ_LIST: printf("[list %p]", obj); break;
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
        case OBJ_LIST: printf("[list %p]", value.obj); break;
        case OBJ_STRING: printf("%s", AS_CSTRING(value)); break;
      }
  }
#endif
}

void pinObj(VM* vm, Obj* obj)
{
  ASSERT(vm->numPinned < MAX_PINNED - 1, "Too many pinned objects.");
  vm->pinned[vm->numPinned++] = obj;
}

void unpinObj(VM* vm, Obj* obj)
{
  ASSERT(vm->pinned[vm->numPinned - 1] == obj, "Unpinning out of stack order.");
  vm->numPinned--;
}
