#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "primitives.h"
#include "vm.h"

static Value primitive_metaclass_new(VM* vm, Fiber* fiber, Value* args);

VM* newVM()
{
  VM* vm = malloc(sizeof(VM));
  initSymbolTable(&vm->methods);
  initSymbolTable(&vm->globalSymbols);

  loadCore(vm);

  return vm;
}

void freeVM(VM* vm)
{
  clearSymbolTable(&vm->methods);
  clearSymbolTable(&vm->globalSymbols);
  free(vm);
}

void initObj(Obj* obj, ObjType type)
{
  obj->type = type;
  obj->flags = 0;
}

Value makeBool(int value)
{
  Obj* obj = malloc(sizeof(Obj));
  initObj(obj, value ? OBJ_TRUE : OBJ_FALSE);
  return obj;
}

ObjClass* makeSingleClass()
{
  ObjClass* obj = malloc(sizeof(ObjClass));
  initObj(&obj->obj, OBJ_CLASS);

  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    obj->methods[i].type = METHOD_NONE;
  }

  return obj;
}

ObjClass* makeClass()
{
  ObjClass* classObj = makeSingleClass();

  // Make its metaclass.
  // TODO(bob): What is the metaclass's metaclass?
  classObj->metaclass = makeSingleClass();

  return classObj;
}

ObjFn* makeFunction()
{
  ObjFn* fn = malloc(sizeof(ObjFn));
  initObj(&fn->obj, OBJ_FN);
  return fn;
}

ObjInstance* makeInstance(ObjClass* classObj)
{
  ObjInstance* instance = malloc(sizeof(ObjInstance));
  initObj(&instance->obj, OBJ_INSTANCE);
  instance->classObj = classObj;

  return instance;
}

Value makeNull()
{
  Obj* obj = malloc(sizeof(Obj));
  initObj(obj, OBJ_NULL);
  return obj;
}

ObjNum* makeNum(double number)
{
  ObjNum* num = malloc(sizeof(ObjNum));
  initObj(&num->obj, OBJ_NUM);
  num->value = number;
  return num;
}

ObjString* makeString(const char* text)
{
  ObjString* string = malloc(sizeof(ObjString));
  initObj(&string->obj, OBJ_STRING);
  string->value = text;
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

// TODO(bob): For debugging. Move to separate module.
/*
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
  // TODO(bob): Allocate fiber on heap.
  Fiber fiber;
  fiber.stackSize = 0;
  fiber.numFrames = 0;

  callFunction(&fiber, fn, 0);

  // These macros are designed to only be invoked within this function.

  // TODO(bob): Check for stack overflow.
  #define PUSH(value) (fiber.stack[fiber.stackSize++] = value)
  #define POP()       (fiber.stack[--fiber.stackSize])
  #define PEEK()      (fiber.stack[fiber.stackSize - 1])
  #define READ_ARG()  (frame->fn->bytecode[frame->ip++])

  for (;;)
  {
    CallFrame* frame = &fiber.frames[fiber.numFrames - 1];

    switch (frame->fn->bytecode[frame->ip++])
    {
      case CODE_CONSTANT:
        PUSH(frame->fn->constants[READ_ARG()]);
        break;

      case CODE_NULL:  PUSH(makeNull()); break;
      case CODE_FALSE: PUSH(makeBool(0)); break;
      case CODE_TRUE:  PUSH(makeBool(1)); break;

      case CODE_CLASS:
      {
        ObjClass* classObj = makeClass();

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
        PUSH(fiber.stack[frame->stackStart + local]);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = READ_ARG();
        fiber.stack[frame->stackStart + local] = PEEK();
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

        Value receiver = fiber.stack[fiber.stackSize - numArgs];

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
            Value* args = &fiber.stack[fiber.stackSize - numArgs];
            Value result = method->primitive(vm, &fiber, args);

            // If the primitive pushed a call frame, it returns NULL.
            if (result != NULL)
            {
              fiber.stack[fiber.stackSize - numArgs] = result;

              // Discard the stack slots for the arguments (but leave one for
              // the result).
              fiber.stackSize -= numArgs - 1;
            }
            break;
          }

          case METHOD_BLOCK:
            callFunction(&fiber, method->fn, numArgs);
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
        PUSH(makeBool(actual == AS_CLASS(classObj)));
        break;
      }

      case CODE_END:
      {
        Value result = POP();
        fiber.numFrames--;

        // If we are returning from the top-level block, just return the value.
        if (fiber.numFrames == 0) return result;

        // Store the result of the block in the first slot, which is where the
        // caller expects it.
        fiber.stack[frame->stackStart] = result;

        // Discard the stack slots for the call frame (leaving one slot for the
        // result).
        fiber.stackSize = frame->stackStart + 1;
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
      printf("[class]");
      break;

    case OBJ_FALSE:
      printf("false");
      break;

    case OBJ_FN:
      printf("[fn]");
      break;

    case OBJ_INSTANCE:
      printf("[instance]");
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

Value primitive_metaclass_new(VM* vm, Fiber* fiber, Value* args)
{
  // TODO(bob): Invoke initializer method.
  return (Value)makeInstance(AS_CLASS(args[0]));
}
