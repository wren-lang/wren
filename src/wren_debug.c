#include <stdio.h>

#include "wren_debug.h"

void wrenDebugPrintStackTrace(WrenVM* vm, ObjFiber* fiber, Value error)
{
  // TODO: Handle error not being a string!
  const char* errorMessage = AS_CSTRING(error);
  fprintf(stderr, "%s\n", errorMessage);

  for (int i = fiber->numFrames - 1; i >= 0; i--)
  {
    CallFrame* frame = &fiber->frames[i];
    ObjFn* fn;
    if (frame->fn->type == OBJ_FN)
    {
      fn = (ObjFn*)frame->fn;
    }
    else
    {
      fn = ((ObjClosure*)frame->fn)->fn;
    }

    // - 1 because IP has advanced past the instruction that it just executed.
    int line = fn->debug->sourceLines[frame->ip - fn->bytecode - 1];
    fprintf(stderr, "[%s line %d] in %s\n",
            fn->debug->sourcePath->value, line, fn->debug->name);
  }
}

static void printLine(ObjFn* fn, int i, int* lastLine)
{
  int line = fn->debug->sourceLines[i];
  if (lastLine == NULL || *lastLine != line)
  {
    printf("%4d:", line);
    if (lastLine != NULL) *lastLine = line;
  }
  else
  {
    printf("     ");
  }
}

static int debugPrintInstruction(WrenVM* vm, ObjFn* fn, int i, int* lastLine)
{
  int start = i;
  uint8_t* bytecode = fn->bytecode;
  Code code = bytecode[i++];

  printLine(fn, i, lastLine);
  printf(" %04d  ", i);

  #define PRINT_ARG \
      printLine(fn, i, lastLine); printf(" %04d    ", ++i); printf

  // TODO: Come up with a cleaner way of displaying 16-bit args.
  
  switch (code)
  {
    case CODE_CONSTANT:
    {
      int constant = (bytecode[i] << 8) | bytecode[i + 1];
      printf("CONSTANT '");
      wrenPrintValue(fn->constants[constant]);
      printf("'\n");
      PRINT_ARG("constant %d\n", constant);
      PRINT_ARG("\n");
      break;
    }

    case CODE_NULL:  printf("NULL\n"); break;
    case CODE_FALSE: printf("FALSE\n"); break;
    case CODE_TRUE:  printf("TRUE\n"); break;

    case CODE_LOAD_LOCAL:
    {
      int local = bytecode[i];
      printf("LOAD_LOCAL %d\n", local);
      PRINT_ARG("local %d\n", local);
      break;
    }

    case CODE_STORE_LOCAL:
    {
      int local = bytecode[i];
      printf("STORE_LOCAL %d\n", local);
      PRINT_ARG("local %d\n", local);
      break;
    }

    case CODE_LOAD_UPVALUE:
    {
      int upvalue = bytecode[i];
      printf("LOAD_UPVALUE %d\n", upvalue);
      PRINT_ARG("upvalue %d\n", upvalue);
      break;
    }

    case CODE_STORE_UPVALUE:
    {
      int upvalue = bytecode[i];
      printf("STORE_UPVALUE %d\n", upvalue);
      PRINT_ARG("upvalue %d\n", upvalue);
      break;
    }

    case CODE_LOAD_GLOBAL:
    {
      int global = bytecode[i];
      printf("LOAD_GLOBAL \"%s\"\n", vm->globalSymbols.names.data[global]);
      PRINT_ARG("global %d\n", global);
      break;
    }

    case CODE_STORE_GLOBAL:
    {
      int global = bytecode[i];
      printf("STORE_GLOBAL \"%s\"\n", vm->globalSymbols.names.data[global]);
      PRINT_ARG("global %d\n", global);
      break;
    }

    case CODE_LOAD_FIELD_THIS:
    {
      int field = bytecode[i];
      printf("LOAD_FIELD_THIS %d\n", field);
      PRINT_ARG("field %d\n", field);
      break;
    }

    case CODE_STORE_FIELD_THIS:
    {
      int field = bytecode[i];
      printf("STORE_FIELD_THIS %d\n", field);
      PRINT_ARG("field %d\n", field);
      break;
    }

    case CODE_LOAD_FIELD:
    {
      int field = bytecode[i];
      printf("LOAD_FIELD %d\n", field);
      PRINT_ARG("field %d\n", field);
      break;
    }

    case CODE_STORE_FIELD:
    {
      int field = bytecode[i];
      printf("STORE_FIELD %d\n", field);
      PRINT_ARG("field %d\n", field);
      break;
    }

    case CODE_POP: printf("POP\n"); break;

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
      int numArgs = bytecode[i - 1] - CODE_CALL_0;
      int symbol = (bytecode[i] << 8) | bytecode[i + 1];
      printf("CALL_%d \"%s\"\n", numArgs, vm->methods.names.data[symbol]);
      PRINT_ARG("symbol %d\n", symbol);
      PRINT_ARG("\n");
      break;
    }

    case CODE_SUPER_0:
    case CODE_SUPER_1:
    case CODE_SUPER_2:
    case CODE_SUPER_3:
    case CODE_SUPER_4:
    case CODE_SUPER_5:
    case CODE_SUPER_6:
    case CODE_SUPER_7:
    case CODE_SUPER_8:
    case CODE_SUPER_9:
    case CODE_SUPER_10:
    case CODE_SUPER_11:
    case CODE_SUPER_12:
    case CODE_SUPER_13:
    case CODE_SUPER_14:
    case CODE_SUPER_15:
    case CODE_SUPER_16:
    {
      int numArgs = bytecode[i - 1] - CODE_SUPER_0;
      int symbol = (bytecode[i] << 8) | bytecode[i + 1];
      printf("SUPER_%d \"%s\"\n", numArgs, vm->methods.names.data[symbol]);
      PRINT_ARG("symbol %d\n", symbol);
      PRINT_ARG("\n");
      break;
    }

    case CODE_JUMP:
    {
      int offset = bytecode[i];
      printf("JUMP %d\n", offset);
      PRINT_ARG("offset %d\n", offset);
      break;
    }

    case CODE_LOOP:
    {
      int offset = bytecode[i];
      printf("LOOP %d\n", offset);
      PRINT_ARG("offset -%d\n", offset);
      break;
    }

    case CODE_JUMP_IF:
    {
      int offset = bytecode[i];
      printf("JUMP_IF %d\n", offset);
      PRINT_ARG("offset %d\n", offset);
      break;
    }

    case CODE_AND:
    {
      int offset = bytecode[i];
      printf("AND %d\n", offset);
      PRINT_ARG("offset %d\n", offset);
      break;
    }

    case CODE_OR:
    {
      int offset = bytecode[i];
      printf("OR %d\n", offset);
      PRINT_ARG("offset %d\n", offset);
      break;
    }

    case CODE_IS:            printf("CODE_IS\n"); break;
    case CODE_CLOSE_UPVALUE: printf("CLOSE_UPVALUE\n"); break;
    case CODE_RETURN:        printf("CODE_RETURN\n"); break;
    case CODE_NEW:           printf("CODE_NEW\n"); break;

    case CODE_LIST:
    {
      int count = bytecode[i];
      printf("LIST\n");
      PRINT_ARG("count %d\n", count);
      break;
    }

    case CODE_CLOSURE:
    {
      int constant = (bytecode[i] << 8) | bytecode[i + 1];
      printf("CLOSURE ");
      wrenPrintValue(fn->constants[constant]);
      printf("\n");
      PRINT_ARG("constant %d\n", constant);
      PRINT_ARG("\n");
      ObjFn* loadedFn = AS_FN(fn->constants[constant]);
      for (int j = 0; j < loadedFn->numUpvalues; j++)
      {
        int isLocal = bytecode[i];
        PRINT_ARG("upvalue %d isLocal %d\n", j, isLocal);
        int index = bytecode[i];
        PRINT_ARG("upvalue %d index %d\n", j, index);
      }
      break;
    }

    case CODE_CLASS:
    {
      int numFields = bytecode[i];
      printf("CLASS\n");
      PRINT_ARG("num fields %d\n", numFields);
      break;
    }

    case CODE_SUBCLASS:
    {
      int numFields = bytecode[i];
      printf("SUBCLASS\n");
      PRINT_ARG("num fields %d\n", numFields);
      break;
    }

    case CODE_METHOD_INSTANCE:
    {
      int symbol = (bytecode[i] << 8) | bytecode[i + 1];
      printf("METHOD_INSTANCE \"%s\"\n", vm->methods.names.data[symbol]);
      PRINT_ARG("symbol %d\n", symbol);
      PRINT_ARG("\n");
      break;
    }

    case CODE_METHOD_STATIC:
    {
      int symbol = (bytecode[i] << 8) | bytecode[i + 1];
      printf("METHOD_STATIC \"%s\"\n", vm->methods.names.data[symbol]);
      PRINT_ARG("symbol %d\n", symbol);
      PRINT_ARG("\n");
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

int wrenDebugPrintInstruction(WrenVM* vm, ObjFn* fn, int i)
{
  return debugPrintInstruction(vm, fn, i, NULL);
}

void wrenDebugPrintCode(WrenVM* vm, ObjFn* fn)
{
  printf("%s: %s\n", fn->debug->sourcePath->value, fn->debug->name);

  int i = 0;
  int lastLine = -1;
  for (;;)
  {
    int offset = debugPrintInstruction(vm, fn, i, &lastLine);
    if (offset == -1) break;
    i += offset;
  }

  printf("\n");
}

void wrenDebugPrintStack(ObjFiber* fiber)
{
  printf(":: ");
  for (int i = 0; i < fiber->stackSize; i++)
  {
    wrenPrintValue(fiber->stack[i]);
    printf(" | ");
  }
  printf("\n");
}
