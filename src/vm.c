#include <stdlib.h>
#include <stdio.h>

#include "vm.h"

static Value pop(Fiber* fiber);

Fiber* newFiber()
{
  Fiber* fiber = (Fiber*)malloc(sizeof(Fiber));
  fiber->stackSize = 0;

  return fiber;
}

Value interpret(Fiber* fiber, Block* block)
{
  int ip = 0;
  for (;;)
  {
    switch (block->bytecode[ip++])
    {
      case CODE_CONSTANT:
      {
        Value value = block->constants[block->bytecode[ip++]];
        fiber->stack[fiber->stackSize++] = value;
        break;
      }

      case CODE_END:
        return pop(fiber);
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
