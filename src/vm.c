#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm.h"

typedef struct
{
  Value stack[STACK_SIZE];
  int stackSize;
} Fiber;

static Value pop(Fiber* fiber);

VM* newVM()
{
  VM* vm = malloc(sizeof(VM));
  vm->numSymbols = 0;

  return vm;
}

void freeVM(VM* vm)
{
  free(vm);
}

int getSymbol(VM* vm, const char* name, size_t length)
{
  // See if the symbol is already defined.
  // TODO(bob): O(n). Do something better.
  for (int i = 0; i < vm->numSymbols; i++)
  {
    if (strncmp(vm->symbols[i], name, length) == 0) return i;
  }

  // New symbol, so add it.
  vm->symbols[vm->numSymbols] = malloc(length);
  strncpy(vm->symbols[vm->numSymbols], name, length);
  return vm->numSymbols++;
}

Value interpret(VM* vm, Block* block)
{
  Fiber fiber;
  fiber.stackSize = 0;

  int ip = 0;
  for (;;)
  {
    switch (block->bytecode[ip++])
    {
      case CODE_CONSTANT:
      {
        Value value = block->constants[block->bytecode[ip++]];
        fiber.stack[fiber.stackSize++] = value;
        break;
      }

      case CODE_CALL:
      {
        int symbol = block->bytecode[ip++];
        printf("call %d\n", symbol);
        break;
      }

      case CODE_END:
        return pop(&fiber);
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

Value pop(Fiber* fiber)
{
  return fiber->stack[--fiber->stackSize];
}
