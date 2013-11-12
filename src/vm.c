#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "primitives.h"
#include "vm.h"

static Value primitive_metaclass_new(VM* vm, Fiber* fiber, Value* args);

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
  vm->nextGC = 1024 * 1024;

  // Clear out the global variables. This ensures they are NULL before being
  // initialized in case we do a garbage collection before one gets initialized.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    vm->globals[i] = NULL;
  }

  // Clear out the pinned list. We look for NULL empty slots explicitly.
  vm->numPinned = 0;
  for (int j = 0; j < MAX_PINNED; j++)
  {
    vm->pinned[j] = NULL;
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

void markObj(Value value)
{
  Obj* obj = (Obj*)value;

  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (obj->flags & FLAG_MARKED) return;

  obj->flags |= FLAG_MARKED;

#ifdef TRACE_MEMORY
  static int indent = 0;
  indent++;
  for (int i = 0; i < indent; i++) printf("  ");
  printf("mark ");
  printValue((Value)obj);
  printf("\n");
#endif

  // Traverse the object's fields.
  switch (obj->type)
  {
    case OBJ_CLASS:
    {
      ObjClass* classObj = AS_CLASS(obj);

      // The metaclass.
      if (classObj->metaclass != NULL) markObj((Value)classObj->metaclass);

      // Method function objects.
      for (int i = 0; i < MAX_SYMBOLS; i++)
      {
        if (classObj->methods[i].type == METHOD_BLOCK)
        {
          markObj((Value)classObj->methods[i].fn);
        }
      }
      break;
    }

    case OBJ_FN:
    {
      // Mark the constants.
      ObjFn* fn = AS_FN(obj);
      for (int i = 0; i < fn->numConstants; i++)
      {
        markObj(fn->constants[i]);
      }
      break;
    }

    case OBJ_INSTANCE:
      // TODO(bob): Mark fields when instances have them.
      break;

    case OBJ_FALSE:
    case OBJ_NULL:
    case OBJ_NUM:
    case OBJ_STRING:
    case OBJ_TRUE:
      // Nothing to mark.
      break;
  }

#ifdef TRACE_MEMORY
  indent--;
#endif
}

void freeObj(VM* vm, Obj* obj)
{
#ifdef TRACE_MEMORY
  printf("free ");
  printValue((Value)obj);
#endif

  // Free any additional heap data allocated by the object.
  size_t size;

  switch (obj->type)
  {
    case OBJ_FN:
    {
      // TODO(bob): Don't hardcode array sizes.
      size = sizeof(ObjFn) + sizeof(Code) * 1024 + sizeof(Value) * 256;
      ObjFn* fn = AS_FN(obj);
      free(fn->bytecode);
      free(fn->constants);
      break;
    }

    case OBJ_STRING:
      // TODO(bob): O(n) calculation here is lame!
      size = sizeof(ObjString) + strlen(AS_STRING(obj));
      free(AS_STRING(obj));
      break;

    case OBJ_CLASS:
      size = sizeof(ObjClass);
      break;

    case OBJ_FALSE:
    case OBJ_INSTANCE:
    case OBJ_NULL:
    case OBJ_NUM:
    case OBJ_TRUE:
      // Nothing to delete.
      size = sizeof(Obj);
      // TODO(bob): Include size of fields for OBJ_INSTANCE.
      break;
  }

  vm->totalAllocated -= size;
#ifdef TRACE_MEMORY
  printf(" (%ld bytes)\n", sizeof(Obj));
#endif

  free(obj);
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
    if (vm->globals[i] != NULL) markObj(vm->globals[i]);
  }

  // Pinned objects.
  for (int j = 0; j < vm->numPinned; j++)
  {
    if (vm->pinned[j] != NULL) markObj(vm->pinned[j]);
  }

  // Stack functions.
  for (int k = 0; k < vm->fiber->numFrames; k++)
  {
    markObj((Value)vm->fiber->frames[k].fn);
  }

  // Stack variables.
  for (int l = 0; l < vm->fiber->stackSize; l++)
  {
    markObj(vm->fiber->stack[l]);
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

void* allocate(VM* vm, size_t size)
{
  vm->totalAllocated += size;

#ifdef DEBUG_GC_STRESS
  collectGarbage(vm);
#else
  if (vm->totalAllocated > vm->nextGC)
  {
#ifdef TRACE_MEMORY
    size_t before = vm->totalAllocated;
#endif
    collectGarbage(vm);
    vm->nextGC = vm->totalAllocated * 3 / 2;

#ifdef TRACE_MEMORY
    printf(
        "GC %ld before, %ld after (%ld collected), next at %ld\n",
        before, vm->totalAllocated, before - vm->totalAllocated, vm->nextGC);
#endif
  }
#endif

  // TODO(bob): Let external code provide allocator.
  return malloc(size);
}

void initObj(VM* vm, Obj* obj, ObjType type)
{
  obj->type = type;
  obj->flags = 0;
  obj->next = vm->first;
  vm->first = obj;
}

Value newBool(VM* vm, int value)
{
  Obj* obj = allocate(vm, sizeof(Obj));
  initObj(vm, obj, value ? OBJ_TRUE : OBJ_FALSE);
  return obj;
}

static ObjClass* newSingleClass(VM* vm)
{
  ObjClass* obj = allocate(vm, sizeof(ObjClass));
  initObj(vm, &obj->obj, OBJ_CLASS);
  obj->metaclass = NULL;

  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    obj->methods[i].type = METHOD_NONE;
  }

  return obj;
}

ObjClass* newClass(VM* vm)
{
  ObjClass* classObj = newSingleClass(vm);

  // Make sure this isn't collected when we allocate the metaclass.
  pinObj(vm, (Value)classObj);

  // Make its metaclass.
  // TODO(bob): What is the metaclass's metaclass?
  classObj->metaclass = newSingleClass(vm);

  unpinObj(vm, (Value)classObj);

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

ObjInstance* newInstance(VM* vm, ObjClass* classObj)
{
  ObjInstance* instance = allocate(vm, sizeof(ObjInstance));
  initObj(vm, &instance->obj, OBJ_INSTANCE);
  instance->classObj = classObj;

  return instance;
}

Value newNull(VM* vm)
{
  Obj* obj = allocate(vm, sizeof(Obj));
  initObj(vm, obj, OBJ_NULL);
  return obj;
}

ObjNum* newNum(VM* vm, double number)
{
  ObjNum* num = allocate(vm, sizeof(ObjNum));
  initObj(vm, &num->obj, OBJ_NUM);
  num->value = number;
  return num;
}

ObjString* newString(VM* vm, const char* text, size_t length)
{
  // Allocate before the string object in case this triggers a GC which would
  // free the string object.
  char* heapText = allocate(vm, length + 1);

  ObjString* string = allocate(vm, sizeof(ObjString));
  initObj(vm, &string->obj, OBJ_STRING);

  // Copy the string (if given one).
  if (text != NULL)
  {
    strncpy(heapText, text, length);
    heapText[length] = '\0';
  }

  string->value = heapText;
  return string;
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

/*
// TODO(bob): For debugging. Move to separate module.
void dumpCode(VM* vm, ObjFn* fn)
{
  unsigned char* bytecode = fn->bytecode;
  int done = 0;
  int i = 0;
  while (!done)
  {
    printf("%04d  ", i);
    unsigned char code = bytecode[i++];

    switch (code)
    {
      case CODE_CONSTANT:
      {
        int constant = bytecode[i++];
        printf("CONSTANT ");
        printValue(fn->constants[constant]);
        printf("\n");
        printf("%04d   | constant %d\n", i - 1, constant);
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
        printf("CLASS\n");
        break;

      case CODE_METACLASS:
        printf("METACLASS\n");
        break;

      case CODE_METHOD:
      {
        int symbol = bytecode[i++];
        int constant = bytecode[i++];
        printf("METHOD \"%s\"\n", getSymbolName(&vm->methods, symbol));
        printf("%04d   | symbol %d\n", i - 2, symbol);
        printf("%04d   | constant %d\n", i - 1, constant);
        break;
      }

      case CODE_LOAD_LOCAL:
      {
        int local = bytecode[i++];
        printf("LOAD_LOCAL %d\n", local);
        printf("%04d   | local %d\n", i - 1, local);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = bytecode[i++];
        printf("STORE_LOCAL %d\n", local);
        printf("%04d   | local %d\n", i - 1, local);
        break;
      }

      case CODE_LOAD_GLOBAL:
      {
        int global = bytecode[i++];
        printf("LOAD_GLOBAL \"%s\"\n",
               getSymbolName(&vm->globalSymbols, global));
        printf("%04d   | global %d\n", i - 1, global);
        break;
      }

      case CODE_STORE_GLOBAL:
      {
        int global = bytecode[i++];
        printf("STORE_GLOBAL \"%s\"\n",
               getSymbolName(&vm->globalSymbols, global));
        printf("%04d   | global %d\n", i - 1, global);
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
        printf("%04d   | symbol %d\n", i - 1, symbol);
        break;
      }

      case CODE_JUMP:
      {
        int offset = bytecode[i++];
        printf("JUMP %d\n", offset);
        printf("%04d   | offset %d\n", i - 1, offset);
        break;
      }

      case CODE_JUMP_IF:
      {
        int offset = bytecode[i++];
        printf("JUMP_IF %d\n", offset);
        printf("%04d   | offset %d\n", i - 1, offset);
        break;
      }

      case CODE_IS:
        printf("CODE_IS\n");
        break;

      case CODE_END:
        printf("CODE_END\n");
        done = 1;
        break;

      default:
        printf("[%d]\n", bytecode[i - 1]);
        break;
    }
  }
}
*/

// Returns the class of [object].
static ObjClass* getClass(VM* vm, Value object)
{
  switch (object->type)
  {
    case OBJ_CLASS: return AS_CLASS(object)->metaclass;
    case OBJ_FALSE:
    case OBJ_TRUE:
      return vm->boolClass;

    case OBJ_FN: return vm->fnClass;
    case OBJ_NULL: return vm->nullClass;
    case OBJ_NUM: return vm->numClass;
    case OBJ_STRING: return vm->stringClass;
    case OBJ_INSTANCE: return AS_INSTANCE(object)->classObj;
  }
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
  #define READ_ARG()  (frame->fn->bytecode[frame->ip++])

  Code lastOp;

  for (;;)
  {
    CallFrame* frame = &fiber->frames[fiber->numFrames - 1];

    if (fiber->stackSize > 0 && PEEK() == NULL)
    {
      lastOp = frame->ip - 1;
      printf("%d\n", lastOp);
    }

    switch (frame->fn->bytecode[frame->ip++])
    {
      case CODE_CONSTANT:
        PUSH(frame->fn->constants[READ_ARG()]);
        break;

      case CODE_NULL:  PUSH(newNull(vm)); break;
      case CODE_FALSE: PUSH(newBool(vm, 0)); break;
      case CODE_TRUE:  PUSH(newBool(vm, 1)); break;

      case CODE_CLASS:
      {
        ObjClass* classObj = newClass(vm);

        // Define a "new" method on the metaclass.
        // TODO(bob): Can this be inherited?
        int newSymbol = ensureSymbol(&vm->methods, "new", strlen("new"));
        classObj->metaclass->methods[newSymbol].type = METHOD_PRIMITIVE;
        classObj->metaclass->methods[newSymbol].primitive =
            primitive_metaclass_new;

        PUSH((Value)classObj);
        break;
      }

      case CODE_METACLASS:
      {
        ObjClass* classObj = AS_CLASS(PEEK());
        PUSH((Value)classObj->metaclass);
        break;
      }

      case CODE_METHOD:
      {
        int symbol = READ_ARG();
        int constant = READ_ARG();
        ObjClass* classObj = AS_CLASS(PEEK());

        ObjFn* body = AS_FN(frame->fn->constants[constant]);
        classObj->methods[symbol].type = METHOD_BLOCK;
        classObj->methods[symbol].fn = body;
        break;
      }

      case CODE_LOAD_LOCAL:
      {
        int local = READ_ARG();
        PUSH(fiber->stack[frame->stackStart + local]);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = READ_ARG();
        fiber->stack[frame->stackStart + local] = PEEK();
        break;
      }

      case CODE_LOAD_GLOBAL:
      {
        int global = READ_ARG();
        PUSH(vm->globals[global]);
        break;
      }

      case CODE_STORE_GLOBAL:
      {
        int global = READ_ARG();
        vm->globals[global] = PEEK();
        break;
      }

      case CODE_DUP: PUSH(PEEK()); break;
      case CODE_POP: POP(); break;

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
        int numArgs = frame->fn->bytecode[frame->ip - 1] - CODE_CALL_0 + 1;
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
            Value result = method->primitive(vm, fiber, args);

            // If the primitive pushed a call frame, it returns NULL.
            if (result != NULL)
            {
              fiber->stack[fiber->stackSize - numArgs] = result;

              // Discard the stack slots for the arguments (but leave one for
              // the result).
              fiber->stackSize -= numArgs - 1;
            }
            break;
          }

          case METHOD_BLOCK:
            callFunction(fiber, method->fn, numArgs);
            break;
        }
        break;
      }

      case CODE_JUMP: frame->ip += READ_ARG(); break;

      case CODE_JUMP_IF:
      {
        int offset = READ_ARG();
        Value condition = POP();

        // False is the only falsey value.
        if (condition->type == OBJ_FALSE)
        {
          frame->ip += offset;
        }
        break;
      }

      case CODE_IS:
      {
        Value classObj = POP();
        Value obj = POP();

        // TODO(bob): What if classObj is not a class?
        ObjClass* actual = getClass(vm, obj);
        PUSH(newBool(vm, actual == AS_CLASS(classObj)));
        break;
      }

      case CODE_END:
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
        break;
      }
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
  // TODO(bob): Do more useful stuff here.
  switch (value->type)
  {
    case OBJ_CLASS:
      printf("[class %p]", value);
      break;

    case OBJ_FALSE:
      printf("false");
      break;

    case OBJ_FN:
      printf("[fn %p]", value);
      break;

    case OBJ_INSTANCE:
      printf("[instance %p]", value);
      break;

    case OBJ_NULL:
      printf("null");
      break;

    case OBJ_NUM:
      printf("%g", AS_NUM(value));
      break;

    case OBJ_STRING:
      printf("%s", AS_STRING(value));
      break;

    case OBJ_TRUE:
      printf("true");
      break;
  }
}

void pinObj(VM* vm, Value value)
{
  ASSERT(vm->numPinned < MAX_PINNED - 1, "Too many pinned objects.");
  vm->pinned[vm->numPinned++] = value;
}

void unpinObj(VM* vm, Value value)
{
  ASSERT(vm->pinned[vm->numPinned - 1] == value,
         "Unpinning object out of stack order.");
  vm->numPinned--;
}

Value primitive_metaclass_new(VM* vm, Fiber* fiber, Value* args)
{
  // TODO(bob): Invoke initializer method.
  return (Value)newInstance(vm, AS_CLASS(args[0]));
}
