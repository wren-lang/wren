#include <stdio.h>

#include "wren_debug.h"

void wrenDebugPrintStackTrace(WrenVM* vm, ObjFiber* fiber)
{
  fprintf(stderr, "%s\n", fiber->error->value);

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

    // Built-in libraries have no source path and are explicitly omitted from
    // stack traces since we don't want to highlight to a user the
    // implementation detail of what part of a core library is implemented in
    // C and what is in Wren.
    if (fn->debug->sourcePath->length == 0) continue;

    // - 1 because IP has advanced past the instruction that it just executed.
    int line = fn->debug->sourceLines[frame->ip - fn->bytecode - 1];
    fprintf(stderr, "[%s line %d] in %s\n",
            fn->debug->sourcePath->value, line, fn->debug->name);
  }
}

static int debugPrintInstruction(WrenVM* vm, ObjFn* fn, int i, int* lastLine)
{
  int start = i;
  uint8_t* bytecode = fn->bytecode;
  Code code = (Code)bytecode[i];

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

  printf(" %04d  ", i++);

  #define READ_BYTE() (bytecode[i++])
  #define READ_SHORT() (i += 2, (bytecode[i - 2] << 8) | bytecode[i - 1])

  #define BYTE_INSTRUCTION(name) \
      printf("%-16s %5d\n", name, READ_BYTE()); \
      break; \

  switch (code)
  {
    case CODE_CONSTANT:
    {
      int constant = READ_SHORT();
      printf("%-16s %5d '", "CONSTANT", constant);
      wrenPrintValue(fn->constants[constant]);
      printf("'\n");
      break;
    }

    case CODE_NULL:  printf("NULL\n"); break;
    case CODE_FALSE: printf("FALSE\n"); break;
    case CODE_TRUE:  printf("TRUE\n"); break;

    case CODE_LOAD_LOCAL_0: printf("LOAD_LOCAL_0\n"); break;
    case CODE_LOAD_LOCAL_1: printf("LOAD_LOCAL_1\n"); break;
    case CODE_LOAD_LOCAL_2: printf("LOAD_LOCAL_2\n"); break;
    case CODE_LOAD_LOCAL_3: printf("LOAD_LOCAL_3\n"); break;
    case CODE_LOAD_LOCAL_4: printf("LOAD_LOCAL_4\n"); break;
    case CODE_LOAD_LOCAL_5: printf("LOAD_LOCAL_5\n"); break;
    case CODE_LOAD_LOCAL_6: printf("LOAD_LOCAL_6\n"); break;
    case CODE_LOAD_LOCAL_7: printf("LOAD_LOCAL_7\n"); break;
    case CODE_LOAD_LOCAL_8: printf("LOAD_LOCAL_8\n"); break;

    case CODE_LOAD_LOCAL: BYTE_INSTRUCTION("LOAD_LOCAL");
    case CODE_STORE_LOCAL: BYTE_INSTRUCTION("STORE_LOCAL");
    case CODE_LOAD_UPVALUE: BYTE_INSTRUCTION("LOAD_UPVALUE");
    case CODE_STORE_UPVALUE: BYTE_INSTRUCTION("STORE_UPVALUE");

    case CODE_LOAD_GLOBAL:
    {
      int global = READ_SHORT();
      printf("%-16s %5d '%s'\n", "LOAD_GLOBAL", global,
             vm->globalNames.data[global]);
      break;
    }

    case CODE_STORE_GLOBAL:
    {
      int global = READ_SHORT();
      printf("%-16s %5d '%s'\n", "STORE_GLOBAL", global,
             vm->globalNames.data[global]);
      break;
    }

    case CODE_LOAD_FIELD_THIS: BYTE_INSTRUCTION("LOAD_FIELD_THIS");
    case CODE_STORE_FIELD_THIS: BYTE_INSTRUCTION("STORE_FIELD_THIS");
    case CODE_LOAD_FIELD: BYTE_INSTRUCTION("LOAD_FIELD");
    case CODE_STORE_FIELD: BYTE_INSTRUCTION("STORE_FIELD");

    case CODE_POP: printf("POP\n"); break;
    case CODE_DUP: printf("DUP\n"); break;

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
      int symbol = READ_SHORT();
      printf("CALL_%-11d %5d '%s'\n", numArgs, symbol,
             vm->methodNames.data[symbol]);
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
      int symbol = READ_SHORT();
      printf("SUPER_%-10d %5d '%s'\n", numArgs, symbol,
             vm->methodNames.data[symbol]);
      break;
    }

    case CODE_JUMP:
    {
      int offset = READ_SHORT();
      printf("%-16s %5d to %d\n", "JUMP", offset, i + offset);
      break;
    }

    case CODE_LOOP:
    {
      int offset = READ_SHORT();
      printf("%-16s %5d to %d\n", "LOOP", offset, i - offset);
      break;
    }

    case CODE_JUMP_IF:
    {
      int offset = READ_SHORT();
      printf("%-16s %5d to %d\n", "JUMP_IF", offset, i + offset);
      break;
    }

    case CODE_AND:
    {
      int offset = READ_SHORT();
      printf("%-16s %5d to %d\n", "AND", offset, i + offset);
      break;
    }

    case CODE_OR:
    {
      int offset = READ_SHORT();
      printf("%-16s %5d to %d\n", "OR", offset, i + offset);
      break;
    }

    case CODE_IS:            printf("CODE_IS\n"); break;
    case CODE_CLOSE_UPVALUE: printf("CLOSE_UPVALUE\n"); break;
    case CODE_RETURN:        printf("CODE_RETURN\n"); break;

    case CODE_CLOSURE:
    {
      int constant = READ_SHORT();
      printf("%-16s %5d ", "CLOSURE", constant);
      wrenPrintValue(fn->constants[constant]);
      printf(" ");
      ObjFn* loadedFn = AS_FN(fn->constants[constant]);
      for (int j = 0; j < loadedFn->numUpvalues; j++)
      {
        int isLocal = READ_BYTE();
        int index = READ_BYTE();
        if (j > 0) printf(", ");
        printf("%s %d", isLocal ? "local" : "upvalue", index);
      }
      printf("\n");
      break;
    }

    case CODE_CLASS:
    {
      int numFields = READ_BYTE();
      printf("%-16s %5d fields\n", "CLASS", numFields);
      break;
    }

    case CODE_METHOD_INSTANCE:
    {
      int symbol = READ_SHORT();
      printf("%-16s %5d '%s'\n", "METHOD_INSTANCE", symbol,
             vm->methodNames.data[symbol]);
      break;
    }

    case CODE_METHOD_STATIC:
    {
      int symbol = READ_SHORT();
      printf("%-16s %5d '%s'\n", "METHOD_STATIC", symbol,
             vm->methodNames.data[symbol]);
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
  printf("(fiber %p) ", fiber);
  for (Value* slot = fiber->stack; slot < fiber->stackTop; slot++)
  {
    wrenPrintValue(*slot);
    printf(" | ");
  }
  printf("\n");
}
