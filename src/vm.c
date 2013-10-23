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
  Block* block;

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

static void callBlock(Fiber* fiber, Block* block, int locals);
static void push(Fiber* fiber, Value value);
static Value pop(Fiber* fiber);
static Value primitiveNumAbs(Value number);

VM* newVM()
{
  VM* vm = malloc(sizeof(VM));
  initSymbolTable(&vm->symbols);

  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    vm->numClass.methods[i] = NULL;
  }

  vm->numClass.methods[ensureSymbol(&vm->symbols, "abs", 3)] = primitiveNumAbs;

  return vm;
}

void freeVM(VM* vm)
{
  clearSymbolTable(&vm->symbols);
  free(vm);
}

Value makeNum(int number)
{
  Value value = malloc(sizeof(Obj));
  value->type = OBJ_INT;
  value->flags = 0;
  value->value = number;

  return value;
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

Value interpret(VM* vm, Block* block)
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

      case CODE_LOAD_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        push(&fiber, fiber.stack[frame->locals + local]);
        break;
      }

      case CODE_STORE_LOCAL:
      {
        int local = frame->block->bytecode[frame->ip++];
        fiber.stack[frame->locals + local] = pop(&fiber);
        break;
      }

      case CODE_POP:
        pop(&fiber);
        break;

      case CODE_CALL:
      {
        Value receiver = pop(&fiber);
        // TODO(bob): Arguments.

        int symbol = frame->block->bytecode[frame->ip++];

        // TODO(bob): Support classes for other object types.
        Class* classObj = &vm->numClass;

        Primitive primitive = classObj->methods[symbol];
        if (primitive)
        {
          Value result = primitive(receiver);
          push(&fiber, result);
        }
        else
        {
          // TODO(bob): Should return nil or suspend fiber or something.
          printf("No method.\n");
          exit(1);
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
    case OBJ_INT:
      printf("%d", value->value);
      break;
  }
}

void callBlock(Fiber* fiber, Block* block, int locals)
{
  fiber->frames[fiber->numFrames].block = block;
  fiber->frames[fiber->numFrames].ip = 0;
  fiber->frames[fiber->numFrames].locals = locals;
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
  int value = number->value;
  if (value < 0) value = -value;

  return makeNum(value);
}
