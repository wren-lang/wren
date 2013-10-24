#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static void callBlock(Fiber* fiber, ObjBlock* block, int firstLocal);
static void push(Fiber* fiber, Value value);
static Value pop(Fiber* fiber);
static Value primitiveNumAbs(Value number);

VM* newVM()
{
  VM* vm = malloc(sizeof(VM));
  initSymbolTable(&vm->symbols);

  vm->numClass = makeClass();
  int symbol = ensureSymbol(&vm->symbols, "abs", 3);
  vm->numClass->methods[symbol].type = METHOD_PRIMITIVE;
  vm->numClass->methods[symbol].primitive = primitiveNumAbs;

  return vm;
}

void freeVM(VM* vm)
{
  clearSymbolTable(&vm->symbols);
  free(vm);
}

ObjClass* makeClass()
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
        printf("load constant ");
        printValue(value);
        printf(" to %d\n", fiber.stackSize - 1);
        break;
      }

      case CODE_CLASS:
        push(&fiber, (Value)makeClass());
        printf("push class at %d\n", fiber.stackSize - 1);
        break;

      case CODE_LOAD_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        push(&fiber, fiber.stack[frame->locals + local]);
        printf("load local %d to %d\n", local, fiber.stackSize - 1);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        printf("store local %d from %d\n", local, fiber.stackSize - 1);
        fiber.stack[frame->locals + local] = fiber.stack[fiber.stackSize - 1];
        break;
      }

      case CODE_DUP:
        push(&fiber, fiber.stack[fiber.stackSize - 1]);
        printf("dup %d\n", fiber.stackSize - 1);
        break;
        
      case CODE_POP:
        printf("pop %d\n", fiber.stackSize - 1);
        pop(&fiber);
        break;

      case CODE_CALL:
      {
        Value receiver = pop(&fiber);
        // TODO(bob): Arguments.

        int symbol = frame->block->bytecode[frame->ip++];

        // TODO(bob): Support classes for other object types.
        ObjClass* classObj = vm->numClass;
        Method* method = &classObj->methods[symbol];
        switch (method->type)
        {
          case METHOD_NONE:
            // TODO(bob): Should return nil or suspend fiber or something.
            printf("No method.\n");
            exit(1);
            break;

          case METHOD_PRIMITIVE:
            push(&fiber, method->primitive(receiver));
            break;

          case METHOD_BLOCK:
            // TODO(bob): Should pass in correct index for locals.
            callBlock(&fiber, method->block, fiber.stackSize);
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
      }
    }
  }
}

void printValue(Value value)
{
  switch (value->type)
  {
    case OBJ_NUM:
      printf("%f", ((ObjNum*)value)->value);
      break;

    case OBJ_BLOCK:
      printf("[block]");
      break;

    case OBJ_CLASS:
      printf("[class]");
      break;
  }
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

Value primitiveNumAbs(Value number)
{
  double value = ((ObjNum*)number)->value;
  if (value < 0) value = -value;

  return (Value)makeNum(value);
}
