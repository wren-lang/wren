#include <stdio.h>

#include "wren_debug.h"

int wrenDebugDumpInstruction(WrenVM* vm, ObjFn* fn, int i)
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
      wrenPrintValue(fn->constants[constant]);
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

    case CODE_LOAD_UPVALUE:
    {
      int upvalue = bytecode[i++];
      printf("LOAD_UPVALUE %d\n", upvalue);
      printf("%04d   | upvalue %d\n", i, upvalue);
      break;
    }

    case CODE_STORE_UPVALUE:
    {
      int upvalue = bytecode[i++];
      printf("STORE_UPVALUE %d\n", upvalue);
      printf("%04d   | upvalue %d\n", i, upvalue);
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
    case CODE_CALL_11:
    case CODE_CALL_12:
    case CODE_CALL_13:
    case CODE_CALL_14:
    case CODE_CALL_15:
    case CODE_CALL_16:
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

    case CODE_CLOSE_UPVALUE:
      printf("CLOSE_UPVALUE\n");
      break;

    case CODE_RETURN:
      printf("CODE_RETURN\n");
      break;

    case CODE_LIST:
    {
      int count = bytecode[i++];
      printf("LIST\n");
      printf("%04d   | count %d\n", i, count);
      break;
    }

    case CODE_CLOSURE:
    {
      int constant = bytecode[i++];
      printf("CLOSURE ");
      wrenPrintValue(fn->constants[constant]);
      printf("\n");
      printf("%04d   | constant %d\n", i, constant);
      ObjFn* loadedFn = AS_FN(fn->constants[constant]);
      for (int j = 0; j < loadedFn->numUpvalues; j++)
      {
        int isLocal = bytecode[i++];
        printf("%04d   | upvalue %d isLocal %d\n", i, j, isLocal);
        int index = bytecode[i++];
        printf("%04d   | upvalue %d index %d\n", i, j, index);
      }
      break;
    }

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

void wrenDebugDumpCode(WrenVM* vm, ObjFn* fn)
{
  int i = 0;
  for (;;)
  {
    int offset = wrenDebugDumpInstruction(vm, fn, i);
    if (offset == -1) break;
    i += offset;
  }
}

void wrenDebugDumpStack(Fiber* fiber)
{
  printf(":: ");
  for (int i = 0; i < fiber->stackSize; i++)
  {
    wrenPrintValue(fiber->stack[i]);
    printf(" | ");
  }
  printf("\n");
}
