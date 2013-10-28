#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "primitives.h"
#include "vm.h"

typedef struct
{
  // Index of the current (really next-to-be-executed) instruction in the
  // block's bytecode.
  int ip;

  // The block being executed.
  ObjBlock* block;

  // Index of the stack slot that contains the first local for this block.
  int locals;
} CallFrame;

typedef struct
{
  Value stack[STACK_SIZE];
  int stackSize;

  CallFrame frames[MAX_CALL_FRAMES];
  int numFrames;
} Fiber;

static Value primitive_metaclass_new(Value* args, int numArgs);
static void callBlock(Fiber* fiber, ObjBlock* block, int firstLocal);
static void push(Fiber* fiber, Value value);
static Value pop(Fiber* fiber);

VM* newVM()
{
  VM* vm = malloc(sizeof(VM));
  initSymbolTable(&vm->symbols);
  initSymbolTable(&vm->globalSymbols);

  // Define the built-in classes.
  vm->blockClass = makeClass();
  vm->classClass = makeClass();
  vm->numClass = makeClass();
  vm->stringClass = makeClass();

  // The call method is special: neither a primitive nor a user-defined one.
  // This is because it mucks with the fiber itself.
  {
    int symbol = ensureSymbol(&vm->symbols, "call", strlen("call"));
    vm->blockClass->methods[symbol].type = METHOD_CALL;
  }

  registerPrimitives(vm);

  return vm;
}

void freeVM(VM* vm)
{
  clearSymbolTable(&vm->symbols);
  free(vm);
}

ObjClass* makeSingleClass()
{
  ObjClass* obj = malloc(sizeof(ObjClass));
  obj->obj.type = OBJ_CLASS;
  obj->obj.flags = 0;

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

ObjBlock* makeBlock()
{
  ObjBlock* block = malloc(sizeof(ObjBlock));
  block->obj.type = OBJ_BLOCK;
  block->obj.flags = 0;
  return block;
}

ObjNum* makeNum(double number)
{
  ObjNum* num = malloc(sizeof(ObjNum));
  num->obj.type = OBJ_NUM;
  num->obj.flags = 0;
  num->value = number;
  return num;
}

ObjString* makeString(const char* text)
{
  ObjString* string = malloc(sizeof(ObjString));
  string->obj.type = OBJ_STRING;
  string->obj.flags = 0;
  string->value = text;
  return string;
}

ObjInstance* makeInstance(ObjClass* classObj)
{
  ObjInstance* instance = malloc(sizeof(ObjInstance));
  instance->obj.type = OBJ_INSTANCE;
  instance->obj.flags = 0;
  instance->classObj = classObj;

  return instance;
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

Value interpret(VM* vm, ObjBlock* block)
{
  Fiber fiber;
  fiber.stackSize = 0;
  fiber.numFrames = 0;

  callBlock(&fiber, block, 0);

  for (;;)
  {
    CallFrame* frame = &fiber.frames[fiber.numFrames - 1];

    switch (frame->block->bytecode[frame->ip++])
    {
      case CODE_CONSTANT:
      {
        int constant = frame->block->bytecode[frame->ip++];
        Value value = frame->block->constants[constant];
        fiber.stack[fiber.stackSize++] = value;
        break;
      }

      case CODE_CLASS:
      {
        ObjClass* classObj = makeClass();

        // Define a "new" method on the metaclass.
        // TODO(bob): Can this be inherited?
        int newSymbol = ensureSymbol(&vm->symbols, "new", strlen("new"));
        classObj->metaclass->methods[newSymbol].type = METHOD_PRIMITIVE;
        classObj->metaclass->methods[newSymbol].primitive =
            primitive_metaclass_new;

        push(&fiber, (Value)classObj);
        break;
      }

      case CODE_METHOD:
      {
        int symbol = frame->block->bytecode[frame->ip++];
        int constant = frame->block->bytecode[frame->ip++];
        ObjClass* classObj = (ObjClass*)fiber.stack[fiber.stackSize - 1];

        ObjBlock* body = (ObjBlock*)frame->block->constants[constant];
        classObj->methods[symbol].type = METHOD_BLOCK;
        classObj->methods[symbol].block = body;
        break;
      }

      case CODE_LOAD_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        push(&fiber, fiber.stack[frame->locals + local]);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        fiber.stack[frame->locals + local] = fiber.stack[fiber.stackSize - 1];
        break;
      }

      case CODE_LOAD_GLOBAL:
      {
        int global = frame->block->bytecode[frame->ip++];
        push(&fiber, vm->globals[global]);
        break;
      }

      case CODE_STORE_GLOBAL:
      {
        int global = frame->block->bytecode[frame->ip++];
        vm->globals[global] = fiber.stack[fiber.stackSize - 1];
        break;
      }

      case CODE_DUP:
        push(&fiber, fiber.stack[fiber.stackSize - 1]);
        break;
        
      case CODE_POP:
        pop(&fiber);
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
        int numArgs = frame->block->bytecode[frame->ip - 1] - CODE_CALL_0;

        Value receiver = fiber.stack[fiber.stackSize - numArgs - 1];
        int symbol = frame->block->bytecode[frame->ip++];

        // TODO(bob): Support classes for other object types.
        ObjClass* classObj;
        switch (receiver->type)
        {
          case OBJ_BLOCK:
            classObj = vm->blockClass;
            break;

          case OBJ_CLASS:
            classObj = ((ObjClass*)receiver)->metaclass;
            break;

          case OBJ_NUM:
            classObj = vm->numClass;
            break;

          case OBJ_STRING:
            classObj = vm->stringClass;
            break;

          case OBJ_INSTANCE:
            classObj = ((ObjInstance*)receiver)->classObj;
            break;
        }

        Method* method = &classObj->methods[symbol];
        switch (method->type)
        {
          case METHOD_NONE:
            // TODO(bob): Should return nil or suspend fiber or something.
            printf("No method.\n");
            exit(1);
            break;

          case METHOD_CALL:
            callBlock(&fiber, (ObjBlock*)receiver, fiber.stackSize - numArgs);
            break;

          case METHOD_PRIMITIVE:
            // TODO(bob): Pass args to primitive.
            fiber.stack[fiber.stackSize - numArgs - 1] =
                method->primitive(&fiber.stack[fiber.stackSize - numArgs - 1],
                                  numArgs);

            // Discard the stack slots for the arguments.
            fiber.stackSize = fiber.stackSize - numArgs;
            break;

          case METHOD_BLOCK:
            callBlock(&fiber, method->block, fiber.stackSize - numArgs);
            break;
        }
        break;
      }

      case CODE_END:
      {
        Value result = pop(&fiber);
        fiber.numFrames--;

        // If we are returning from the top-level block, just return the value.
        if (fiber.numFrames == 0) return result;

        // Store the result of the block in the first slot, which is where the
        // caller expects it.
        fiber.stack[frame->locals] = result;

        // Discard the stack slots for the locals.
        fiber.stackSize = frame->locals + 1;
      }
    }
  }
}

void printValue(Value value)
{
  // TODO(bob): Do more useful stuff here.
  switch (value->type)
  {
    case OBJ_NUM:
      printf("%g", ((ObjNum*)value)->value);
      break;

    case OBJ_STRING:
      printf("%s", ((ObjString*)value)->value);
      break;

    case OBJ_BLOCK:
      printf("[block]");
      break;

    case OBJ_CLASS:
      printf("[class]");
      break;

    case OBJ_INSTANCE:
      printf("[instance]");
      break;
  }
}

Value primitive_metaclass_new(Value* args, int numArgs)
{
  ObjClass* classObj = (ObjClass*)args[0];
  // TODO(bob): Invoke initializer method.
  return (Value)makeInstance(classObj);
}

void callBlock(Fiber* fiber, ObjBlock* block, int firstLocal)
{
  fiber->frames[fiber->numFrames].block = block;
  fiber->frames[fiber->numFrames].ip = 0;
  fiber->frames[fiber->numFrames].locals = firstLocal;

  // Make empty slots for locals.
  // TODO(bob): Don't push slots for params since the args are already there.
  // TODO(bob): Should we push some real nil value here?
  for (int i = 0; i < block->numLocals; i++)
  {
    push(fiber, NULL);
  }

  fiber->numFrames++;
}

void push(Fiber* fiber, Value value)
{
  // TODO(bob): Check for stack overflow.
  fiber->stack[fiber->stackSize++] = value;
}

Value pop(Fiber* fiber)
{
  return fiber->stack[--fiber->stackSize];
}
