#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "primitives.h"
#include "vm.h"

static Value primitive_metaclass_new(VM* vm, Value* args);

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
  // TODO(bob): Mark fields when instances have them.
}

static void markObj(Obj* obj)
{
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
    case OBJ_CLASS: markClass((ObjClass*)obj); break;
    case OBJ_FN: markFn((ObjFn*)obj); break;
    case OBJ_INSTANCE: markInstance((ObjInstance*)obj); break;
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
      ObjFn* fn = (ObjFn*)obj;
      free(fn->bytecode);
      free(fn->constants);
      break;
    }

    case OBJ_STRING:
    {
      // TODO(bob): O(n) calculation here is lame!
      ObjString* string = (ObjString*)obj;
      size = sizeof(ObjString) + strlen(string->value);
      free(string->value);
      break;
    }

    case OBJ_CLASS:
      size = sizeof(ObjClass);
      break;

    case OBJ_INSTANCE:
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

static ObjClass* newSingleClass(VM* vm, ObjClass* metaclass,
                                ObjClass* superclass)
{
  ObjClass* obj = allocate(vm, sizeof(ObjClass));
  initObj(vm, &obj->obj, OBJ_CLASS);
  obj->metaclass = metaclass;
  obj->superclass = superclass;

  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    obj->methods[i].type = METHOD_NONE;
  }

  return obj;
}

ObjClass* newClass(VM* vm, ObjClass* superclass)
{
  // Make the metaclass.
  // TODO(bob): What is the metaclass's metaclass and superclass?
  ObjClass* metaclass = newSingleClass(vm, NULL, NULL);

  // Make sure it isn't collected when we allocate the metaclass.
  pinObj(vm, (Obj*)metaclass);

  ObjClass* classObj = newSingleClass(vm, metaclass, superclass);

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
  ObjInstance* instance = allocate(vm, sizeof(ObjInstance));
  initObj(vm, &instance->obj, OBJ_INSTANCE);
  instance->classObj = classObj;

  return OBJ_VAL(instance);
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
      printf("CLASS\n");
      break;

    case CODE_SUBCLASS:
      printf("SUBCLASS\n");
      break;

    case CODE_METACLASS:
      printf("METACLASS\n");
      break;

    case CODE_METHOD:
    {
      int symbol = bytecode[i++];
      int constant = bytecode[i++];
      printf("METHOD \"%s\"\n", getSymbolName(&vm->methods, symbol));
      printf("%04d   | symbol %d\n", i - 1, symbol);
      printf("%04d   | constant %d\n", i, constant);
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

  for (;;)
  {
    Code instruction = bytecode[ip++];
    switch (instruction)
    {
      case CODE_CONSTANT:
        PUSH(frame->fn->constants[READ_ARG()]);
        break;

      case CODE_NULL:  PUSH(NULL_VAL); break;
      case CODE_FALSE: PUSH(FALSE_VAL); break;
      case CODE_TRUE:  PUSH(TRUE_VAL); break;

      case CODE_CLASS:
      case CODE_SUBCLASS:
      {
        int isSubclass = instruction == CODE_SUBCLASS;

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

        ObjClass* classObj = newClass(vm, superclass);

        // Assume the first class being defined is Object.
        if (vm->objectClass == NULL)
        {
          vm->objectClass = classObj;
        }

        // Define a "new" method on the metaclass.
        // TODO(bob): Can this be inherited?
        int newSymbol = ensureSymbol(&vm->methods, "new", strlen("new"));
        classObj->metaclass->methods[newSymbol].type = METHOD_PRIMITIVE;
        classObj->metaclass->methods[newSymbol].primitive =
            primitive_metaclass_new;

        PUSH(OBJ_VAL(classObj));
        break;
      }

      case CODE_METACLASS:
      {
        ObjClass* classObj = AS_CLASS(PEEK());
        PUSH(OBJ_VAL(classObj->metaclass));
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
            // Store the IP back into the frame.
            frame->ip = ip;

            Value* args = &fiber->stack[fiber->stackSize - numArgs];
            method->fiberPrimitive(vm, fiber, args);

            // These have changed now, so update them.
            frame = &fiber->frames[fiber->numFrames - 1];
            ip = frame->ip;
            bytecode = frame->fn->bytecode;
            break;
          }
            
          case METHOD_BLOCK:
            // Store the IP back into the frame.
            frame->ip = ip;

            callFunction(fiber, method->fn, numArgs);

            // These have changed now, so update them.
            frame = &fiber->frames[fiber->numFrames - 1];
            ip = frame->ip;
            bytecode = frame->fn->bytecode;
            break;
        }
        break;
      }

      case CODE_JUMP:
      {
        int offset = READ_ARG();
        ip += offset;
        break;
      }

      case CODE_LOOP:
      {
        // The loop body's result is on the top of the stack. Since we are
        // looping and running the body again, discard it.
        POP();

        // Jump back to the top of the loop.
        int offset = READ_ARG();
        ip -= offset;
        break;
      }

      case CODE_JUMP_IF:
      {
        int offset = READ_ARG();
        Value condition = POP();

        // False is the only falsey value.
        if (IS_FALSE(condition))
        {
          ip += offset;
        }
        break;
      }

      case CODE_AND:
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
        break;
      }

      case CODE_OR:
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
        break;
      }

      case CODE_IS:
      {
        Value classObj = POP();
        Value obj = POP();

        // TODO(bob): What if classObj is not a class?
        ObjClass* actual = getClass(vm, obj);
        PUSH(BOOL_VAL(actual == AS_CLASS(classObj)));
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

        // These have changed now, so update them.
        frame = &fiber->frames[fiber->numFrames - 1];
        ip = frame->ip;
        bytecode = frame->fn->bytecode;
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
  // TODO(bob): Unify these.
#ifdef NAN_TAGGING
  if (IS_NUM(value))
  {
    printf("%g", AS_NUM(value));
  }
  else if (IS_OBJ(value))
  {
    Obj* obj = AS_OBJ(value);
    switch (obj->type)
    {
      case OBJ_CLASS: printf("[class %p]", obj); break;
      case OBJ_FN: printf("[fn %p]", obj); break;
      case OBJ_INSTANCE: printf("[instance %p]", obj); break;
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
    case VAL_NUM: printf("%g", AS_NUM(value)); break;
    case VAL_TRUE: printf("true"); break;
    case VAL_OBJ:
      switch (value.obj->type)
      {
        case OBJ_CLASS: printf("[class %p]", value.obj); break;
        case OBJ_FN: printf("[fn %p]", value.obj); break;
        case OBJ_INSTANCE: printf("[instance %p]", value.obj); break;
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

Value primitive_metaclass_new(VM* vm, Value* args)
{
  // TODO(bob): Invoke initializer method.
  return newInstance(vm, AS_CLASS(args[0]));
}
