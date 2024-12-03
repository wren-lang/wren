#include <stdio.h>

#include "wren_debug.h"

void wrenDebugPrintStackTrace(WrenVM* vm)
{
  // Bail if the host doesn't enable printing errors.
  if (vm->config.errorFn == NULL) return;
  
  ObjFiber* fiber = vm->fiber;
  if (IS_STRING(fiber->error))
  {
    vm->config.errorFn(vm, WREN_ERROR_RUNTIME,
                       NULL, -1, AS_CSTRING(fiber->error));
  }
  else
  {
    // TODO: Print something a little useful here. Maybe the name of the error's
    // class?
    vm->config.errorFn(vm, WREN_ERROR_RUNTIME,
                       NULL, -1, "[error object]");
  }

  for (int i = fiber->numFrames - 1; i >= 0; i--)
  {
    CallFrame* frame = &fiber->frames[i];
    ObjFn* fn = frame->closure->fn;

    // Skip over stub functions for calling methods from the C API.
    if (fn->module == NULL) continue;
    
    // The built-in core module has no name. We explicitly omit it from stack
    // traces since we don't want to highlight to a user the implementation
    // detail of what part of the core module is written in C and what is Wren.
    if (fn->module->name == NULL) continue;
    
    // -1 because IP has advanced past the instruction that it just executed.
    int line = fn->debug->sourceLines.count
      ? fn->debug->sourceLines.data[frame->ip - fn->code.data - 1]
      : 0;
    vm->config.errorFn(vm, WREN_ERROR_STACK_TRACE,
                       fn->module->name->value, line,
                       fn->debug->name);
  }
}

static void dumpObject(FILE* stream, Obj* obj, bool withType)
{
  switch (obj->type)
  {
    case OBJ_CLASS:
      fprintf(stream, "[class %s %p]", ((ObjClass*)obj)->name->value, obj);
      break;
    case OBJ_CLOSURE: fprintf(stream, "[closure %p]", obj); break;
    case OBJ_FIBER: fprintf(stream, "[fiber %p]", obj); break;
    case OBJ_FN: fprintf(stream, "[fn %p]", obj); break;
    case OBJ_FOREIGN: fprintf(stream, "[foreign %p]", obj); break;
    case OBJ_INSTANCE: fprintf(stream, "[instance %p]", obj); break;
    case OBJ_LIST: fprintf(stream, "[list %p]", obj); break;
    case OBJ_MAP: fprintf(stream, "[map %p]", obj); break;
    case OBJ_MODULE: fprintf(stream, "[module %p]", obj); break;
    case OBJ_RANGE: fprintf(stream, "[range %p]", obj); break;
    case OBJ_STRING:
      if (withType) fprintf(stream, "STR:\"");
      fprintf(stream, "%s", ((ObjString*)obj)->value);
      if (withType) fprintf(stream, "\"");
      break;
    case OBJ_UPVALUE: fprintf(stream, "[upvalue %p]", obj); break;
    default: fprintf(stream, "[unknown object %d]", obj->type); break;
  }
}

static void wrenDumpValue_(FILE* stream, Value value, bool withType)
{
#if WREN_NAN_TAGGING
  if (IS_NUM(value))
  {
    if (withType) fprintf(stream, "NUM:");
    fprintf(stream, "%.14g", AS_NUM(value));
  }
  else if (IS_OBJ(value))
  {
    dumpObject(stream, AS_OBJ(value), withType);
  }
  else
  {
    switch (GET_TAG(value))
    {
      case TAG_FALSE:     fprintf(stream, "false"); break;
      case TAG_NAN:       fprintf(stream, "NaN"); break;
      case TAG_NULL:      fprintf(stream, "null"); break;
      case TAG_TRUE:      fprintf(stream, "true"); break;
      case TAG_UNDEFINED: UNREACHABLE();
    }
  }
#else
  switch (value.type)
  {
    case VAL_FALSE:     fprintf(stream, "false"); break;
    case VAL_NULL:      fprintf(stream, "null"); break;
    case VAL_NUM:
      if (withType) fprintf(stream, "NUM:");
      fprintf(stream, "%.14g", AS_NUM(value));
      break;
    case VAL_TRUE:      fprintf(stream, "true"); break;
    case VAL_OBJ:       dumpObject(stream, AS_OBJ(value), withType); break;
    case VAL_UNDEFINED: UNREACHABLE();
  }
#endif
}

void wrenDumpValue(Value value)
{
  wrenDumpValue_(stdout, value, false);
}

static int dumpInstruction(WrenVM* vm, ObjFn* fn, int i, int* lastLine)
{
  int start = i;
  uint8_t* bytecode = fn->code.data;
  Code code = (Code)bytecode[i];

  int line = fn->debug->sourceLines.count ? fn->debug->sourceLines.data[i] : 0;
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

  #define BYTE_INSTRUCTION(name)                                               \
      printf("%-16s %5d\n", name, READ_BYTE());                                \
      break

  switch (code)
  {
    case CODE_CONSTANT:
    {
      int constant = READ_SHORT();
      printf("%-16s %5d '", "CONSTANT", constant);
      wrenDumpValue(fn->constants.data[constant]);
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

    case CODE_LOAD_MODULE_VAR:
    {
      int slot = READ_SHORT();
      printf("%-16s %5d '%s'\n", "LOAD_MODULE_VAR", slot,
             fn->module->variableNames.data[slot]->value);
      break;
    }

    case CODE_STORE_MODULE_VAR:
    {
      int slot = READ_SHORT();
      printf("%-16s %5d '%s'\n", "STORE_MODULE_VAR", slot,
             fn->module->variableNames.data[slot]->value);
      break;
    }

    case CODE_LOAD_FIELD_THIS: BYTE_INSTRUCTION("LOAD_FIELD_THIS");
    case CODE_STORE_FIELD_THIS: BYTE_INSTRUCTION("STORE_FIELD_THIS");
    case CODE_LOAD_FIELD: BYTE_INSTRUCTION("LOAD_FIELD");
    case CODE_STORE_FIELD: BYTE_INSTRUCTION("STORE_FIELD");

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
      int numArgs = code - CODE_CALL_0;
      int symbol = READ_SHORT();
      printf("CALL_%-11d %5d '%s'\n", numArgs, symbol,
             vm->methodNames.data[symbol]->value);
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
      int numArgs = code - CODE_SUPER_0;
      int symbol = READ_SHORT();
      int superclass = READ_SHORT();
      printf("SUPER_%-10d %5d '%s' %5d\n", numArgs, symbol,
             vm->methodNames.data[symbol]->value, superclass);
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

    case CODE_CLOSE_UPVALUE: printf("CLOSE_UPVALUE\n"); break;
    case CODE_RETURN:        printf("RETURN\n"); break;

    case CODE_CLOSURE:
    {
      int constant = READ_SHORT();
      printf("%-16s %5d ", "CLOSURE", constant);
      wrenDumpValue(fn->constants.data[constant]);
      ObjFn* loadedFn = AS_FN(fn->constants.data[constant]);
      for (int j = 0; j < loadedFn->numUpvalues; j++)
      {
        int isLocal = READ_BYTE();
        int index = READ_BYTE();
        if (j > 0) printf(", "); else printf(" ");
        printf("%s %d", isLocal ? "local" : "upvalue", index);
      }
      printf("\n");
      break;
    }

    case CODE_CONSTRUCT:         printf("CONSTRUCT\n"); break;
    case CODE_FOREIGN_CONSTRUCT: printf("FOREIGN_CONSTRUCT\n"); break;
      
    case CODE_CLASS:
    {
      int numFields = READ_BYTE();
      printf("%-16s %5d fields\n", "CLASS", numFields);
      break;
    }

    case CODE_FOREIGN_CLASS: printf("FOREIGN_CLASS\n"); break;
    case CODE_END_CLASS: printf("END_CLASS\n"); break;

    case CODE_METHOD_INSTANCE:
    {
      int symbol = READ_SHORT();
      printf("%-16s %5d '%s'\n", "METHOD_INSTANCE", symbol,
             vm->methodNames.data[symbol]->value);
      break;
    }

    case CODE_METHOD_STATIC:
    {
      int symbol = READ_SHORT();
      printf("%-16s %5d '%s'\n", "METHOD_STATIC", symbol,
             vm->methodNames.data[symbol]->value);
      break;
    }
      
    case CODE_END_MODULE: printf("END_MODULE\n"); break;
      
    case CODE_IMPORT_MODULE:
    {
      int name = READ_SHORT();
      printf("%-16s %5d '", "IMPORT_MODULE", name);
      wrenDumpValue(fn->constants.data[name]);
      printf("'\n");
      break;
    }
      
    case CODE_IMPORT_VARIABLE:
    {
      int variable = READ_SHORT();
      printf("%-16s %5d '", "IMPORT_VARIABLE", variable);
      wrenDumpValue(fn->constants.data[variable]);
      printf("'\n");
      break;
    }
      
    case CODE_END: printf("END\n"); break;

    default:
      printf("UNKNOWN! [%d]\n", code);
      break;
  }

  // Return how many bytes this instruction takes, or -1 if it's an END.
  if (code == CODE_END) return -1;
  return i - start;

  #undef READ_BYTE
  #undef READ_SHORT
  #undef BYTE_INSTRUCTION
}

int wrenDumpInstruction(WrenVM* vm, ObjFn* fn, int i)
{
  return dumpInstruction(vm, fn, i, NULL);
}

void wrenDumpCode(WrenVM* vm, ObjFn* fn)
{
  printf("%s: %s\n",
         fn->module->name == NULL ? "<core>" : fn->module->name->value,
         fn->debug->name);

  int i = 0;
  int lastLine = -1;
  for (;;)
  {
    int offset = dumpInstruction(vm, fn, i, &lastLine);
    if (offset == -1) break;
    i += offset;
  }

  printf("\n");
}

void wrenDumpStack(ObjFiber* fiber)
{
  printf("(fiber %p) ", fiber);
  for (Value* slot = fiber->stack; slot < fiber->stackTop; slot++)
  {
    wrenDumpValue(*slot);
    printf(" | ");
  }
  printf("\n");
}

// Snapshot --------------------------------------------------------------------

typedef struct {
  FILE* file;
  WrenCounts *counts;
  WrenCensus *census;
} WrenSnapshotContext;

#define CHAR(oneCharStr) fwrite(oneCharStr, sizeof(char),    1, file)
#define STR_CONST(str)   fwrite(str,        sizeof(str) - 1, 1, file)
#define STR(str)         fwrite(str,        strlen(str),     1, file)
#define NUM(n)           fwrite(&n,         sizeof(n),       1, file)
  // TODO check returned values

static const bool verbose = true;

#define VERBOSE    if (verbose)

// How to serialize the type of an Obj or a Value.
typedef uint8_t ObjOrValueType;

// A char to discriminate the type of a Value.
// Purposefully don't conflict with the numbers defined by enum ObjType.
enum {
  ValueTypeCharFalse    = '0',
  ValueTypeCharNull     = ' ',
  ValueTypeCharNum      = 'N',
  ValueTypeCharTrue     = '1',
  ValueTypeCharNaN      = '8', // 8 is not an infinite :-)
};

static void saveValue(FILE* file, WrenCounts* counts, WrenCensus* census, Value v)
{
#if WREN_NAN_TAGGING
  if (IS_NUM(v))
  {
    const ObjOrValueType type = ValueTypeCharNum;
    double d = AS_NUM(v);
    NUM(type);
    NUM(d);                       // TODO portability?
  }
  else if (IS_OBJ(v))
  {
    Obj* obj = AS_OBJ(v);
    const ObjOrValueType type = obj->type;   // NOTE the cast
    WrenCount id = wrenFindInCensus(counts, census, obj);

    NUM(type);
    NUM(id);
  }
  else
  {
    ObjOrValueType type;
    switch (GET_TAG(v))
    {
      case TAG_FALSE:     type = ValueTypeCharFalse; break;
      case TAG_NAN:       type = ValueTypeCharNaN;   break;
      case TAG_NULL:      type = ValueTypeCharNull;  break;
      case TAG_TRUE:      type = ValueTypeCharTrue;  break;
      case TAG_UNDEFINED: UNREACHABLE();
    }
    NUM(type);
  }
#else
  ObjOrValueType type;
  switch (v.type)
  {
    case VAL_FALSE: type = ValueTypeCharFalse; NUM(type); break;
    case VAL_NULL:  type = ValueTypeCharNull;  NUM(type); break;
    case VAL_TRUE:  type = ValueTypeCharTrue;  NUM(type); break;
    case VAL_NUM: {
      type = ValueTypeCharNum;
      double d = AS_NUM(v);
      NUM(type);
      NUM(d);                       // TODO portability?
      break;
    }
    case VAL_OBJ: {
      Obj* obj = AS_OBJ(v);
      type = obj->type;             // NOTE the cast
      WrenCount id = wrenFindInCensus(counts, census, obj);

      NUM(type);
      NUM(id);
      break;
    }
    case VAL_UNDEFINED: UNREACHABLE();
  }
#endif
}

static void saveValueBuffer(FILE* file, WrenCounts* counts, WrenCensus* census, ValueBuffer* buffer)
{
  const int count = buffer->count;
  Value* data = buffer->data;

  NUM(count);
  VERBOSE CHAR("{");
  for (int i = 0; i < count; ++i)
  {
    Value v = data[i];

    if (i)
    {
      VERBOSE CHAR(",");
    }

    saveValue(file, counts, census, v);
  }
  VERBOSE CHAR("}");
}

static void saveStringBuffer(FILE* file, WrenCounts* counts, WrenCensus* census, StringBuffer* buffer)
{
  const int count = buffer->count;
  ObjString** data = buffer->data;

  NUM(count);
  VERBOSE CHAR("{");
  for (int i = 0; i < count; ++i)
  {
    ObjString* str = data[i];

    if (i)
    {
      VERBOSE CHAR(",");
    }

    WrenCount id = wrenFindInCensus(counts, census, (Obj*)str);
    NUM(id);
  }
  VERBOSE CHAR("}");
}

enum {
  MethodTypeCharPrimitive     = 'P',
  MethodTypeCharFunctionCall  = 'C',  // call
  MethodTypeCharForeign       = 'F',
  MethodTypeCharBlock         = 'M',  // method
  MethodTypeCharNone          = ' ',
};

static void saveMethodBuffer(FILE* file, WrenCounts* counts, WrenCensus* census, MethodBuffer* buffer)
{
  const int count = buffer->count;
  Method* data = buffer->data;

  NUM(count);
  VERBOSE CHAR("{");
  for (int i = 0; i < count; ++i)
  {
    Method m = data[i];

    if (i)
    {
      VERBOSE CHAR(",");
    }

    uint8_t type;             // NOTE the type
    switch (m.type)
    {
      case METHOD_PRIMITIVE:
        type = MethodTypeCharPrimitive;
        NUM(type);
        // TODO id for a Primitive
        break;

      case METHOD_FUNCTION_CALL:
        type = MethodTypeCharFunctionCall;
        NUM(type);
        // TODO id for a Primitive
        break;

      case METHOD_FOREIGN:
        type = MethodTypeCharForeign;
        NUM(type);
        // TODO id for a WrenForeignMethodFn
        break;

      case METHOD_BLOCK:
      {
        type = MethodTypeCharBlock;
        NUM(type);

        WrenCount id = wrenFindInCensus(counts, census, (Obj*)m.as.closure);
        NUM(id);
        break;
      }

      case METHOD_NONE:
        type = MethodTypeCharNone;
        NUM(type);
        break;
    }
  }
  VERBOSE CHAR("}");
}

static void saveByteBuffer(FILE* file, WrenCounts* counts, WrenCensus* census, ByteBuffer* buffer)
{
  const int count = buffer->count;
  const uint8_t* data = buffer->data;

  NUM(count);
  VERBOSE CHAR("{");
  fwrite(data, sizeof(uint8_t), count, file);
  VERBOSE CHAR("}");
}

static void saveObjString(FILE* file, WrenCounts* counts, WrenCensus* census, ObjString* str)
{
  uint32_t length = str->length;
  NUM(length);
  VERBOSE CHAR("\"");
  fwrite(str->value, sizeof(char), length, file);
  VERBOSE CHAR("\"");
}

static void saveObjModule(FILE* file, WrenCounts* counts, WrenCensus* census, ObjModule* module)
{
  // The core module has no name, hence id_name will be 0.
  ObjString* name = module->name;
  WrenCount id_name = wrenFindInCensus(counts, census, (Obj*)name);
  NUM(id_name);

  saveStringBuffer(file, counts, census, (StringBuffer*) &module->variableNames);
  saveValueBuffer(file, counts, census, &module->variables);
}

static void saveObjFn(FILE* file, WrenCounts* counts, WrenCensus* census, ObjFn* fn)
{
  const ObjModule* module = fn->module;
  WrenCount id_module = wrenFindInCensus(counts, census, (Obj*)module);
  NUM(id_module);

  VERBOSE CHAR(":");
  VERBOSE CHAR(":");

  const uint64_t lenName = strlen(fn->debug->name);   // NOTE the type
  NUM(lenName);
  STR(fn->debug->name);

  VERBOSE CHAR("/");
  const uint8_t arity = fn->arity;    // NOTE the type; see MAX_PARAMETERS
  NUM(arity);

  VERBOSE CHAR("S");
  NUM(fn->maxSlots); // TODO endianness, type

  VERBOSE CHAR("U");
  const uint8_t numUpvalues = fn->numUpvalues;    // NOTE the type; see MAX_UPVALUES
  NUM(numUpvalues);

  VERBOSE CHAR("C");
  saveValueBuffer(file, counts, census, &fn->constants);

  VERBOSE CHAR("B");
  saveByteBuffer(file, counts, census, &fn->code);

  // TODO FnDebug* debug->sourceLines
  // const int sourceLinesCount = fn->debug->sourceLines.count;
  // VERBOSE CHAR("D");
  // NUM(sourceLinesCount);
}

static void saveObjClosure(FILE* file, WrenCounts* counts, WrenCensus* census, ObjClosure* closure)
{
  const ObjFn* fn = closure->fn;
  WrenCount id_fn = wrenFindInCensus(counts, census, (Obj*)fn);
  NUM(id_fn);

  // TODO upvalues, fn->numUpvalues
}

static void saveObjMap(FILE* file, WrenCounts* counts, WrenCensus* census, ObjMap* map)
{
  const uint32_t count = map->count;

  if (count)
  {
    NUM(count);
    VERBOSE CHAR("{");

    uint32_t i = 0;
    for (uint32_t index = 0; index < map->capacity; index++)
    {
      MapEntry* entry = &map->entries[index];

      if (IS_UNDEFINED(entry->key)) continue;

      if (i++)
      {
        VERBOSE CHAR(",");
      }

      saveValue(file, counts, census, entry->key);
      VERBOSE CHAR("=");
      VERBOSE CHAR(">");
      saveValue(file, counts, census, entry->value);
    }
    VERBOSE CHAR("}");
  }
}

static void saveObjClass(FILE* file, WrenCounts* counts, WrenCensus* census, ObjClass* classObj)
{
  ObjString* name = classObj->name;
  WrenCount id_name = wrenFindInCensus(counts, census, (Obj*)name);
  NUM(id_name);

  VERBOSE CHAR("<");
  ObjClass* super = classObj->superclass;
  WrenCount id_super = wrenFindInCensus(counts, census, (Obj*)super);
  NUM(id_super);

  VERBOSE CHAR("A");
  saveValue(file, counts, census, classObj->attributes);

  VERBOSE CHAR("F");
  const uint8_t numFields = classObj->numFields;    // NOTE the type; see MAX_FIELDS
  NUM(numFields);

  VERBOSE CHAR("M");
  saveMethodBuffer(file, counts, census, &classObj->methods);
}

#define SAVE_ALL(type)                                                         \
static void saveAll##type(FILE* file, WrenCounts* counts, WrenCensus* census)  \
{                                                                              \
  static const char strType[] = "Obj" #type;                                   \
                                                                               \
  const WrenCount nb = counts->nb##type;                                       \
  Obj##type** all = census->all##type;                                         \
                                                                               \
  VERBOSE STR_CONST(strType);                                                  \
  VERBOSE CHAR("=");                                                           \
  NUM(nb);                                                                     \
  VERBOSE CHAR("\n");                                                          \
  for (WrenCount i = 0; i < nb; ++i)                                           \
  {                                                                            \
    WrenCount id = i + 1;                                                      \
    Obj##type* obj = all[i];                                                   \
                                                                               \
    VERBOSE STR_CONST(strType);                                                \
    VERBOSE CHAR("#");                                                         \
    VERBOSE NUM(id);                                                           \
    VERBOSE CHAR(":");                                                         \
                                                                               \
    saveObj##type(file, counts, census, obj);                                  \
                                                                               \
    VERBOSE CHAR("\n");                                                        \
  }                                                                            \
}

SAVE_ALL(String)
SAVE_ALL(Module)
SAVE_ALL(Fn)
SAVE_ALL(Closure)
SAVE_ALL(Map)
SAVE_ALL(Class)

#undef SAVE_ALL

static void saveVM(FILE* file, WrenCounts* counts, WrenCensus* census, WrenVM* vm)
{
  VERBOSE CHAR("V");
  VERBOSE CHAR("M");
  VERBOSE CHAR("\n");

#define SAVE_CLASS(verboseCharStr, name)                                       \
  VERBOSE CHAR(verboseCharStr);                                                \
  ObjClass* name##Class = vm->name##Class;                                     \
  const WrenCount id_##name##Class =                                           \
    wrenFindInCensus(counts, census, (Obj*)name##Class);                       \
  NUM(id_##name##Class);                                                       \
  VERBOSE CHAR("\n");

  SAVE_CLASS("b", bool)
  SAVE_CLASS("c", class)
  SAVE_CLASS("t", fiber)    // thread
  SAVE_CLASS("f", fn)
  SAVE_CLASS("l", list)
  SAVE_CLASS("m", map)
  SAVE_CLASS(" ", null)
  SAVE_CLASS("n", num)
  SAVE_CLASS("o", object)
  SAVE_CLASS("r", range)
  SAVE_CLASS("s", string)

#undef SAVE_CLASS

  VERBOSE CHAR("i");  // import
  const WrenCount id_modules = wrenFindInCensus(counts, census, (Obj*)vm->modules);
  NUM(id_modules);
  VERBOSE CHAR("\n");

  VERBOSE CHAR("I");  // import
  const WrenCount id_lastModule = wrenFindInCensus(counts, census, (Obj*)vm->lastModule);
  NUM(id_lastModule);
  VERBOSE CHAR("\n");

  VERBOSE CHAR("M");
  saveStringBuffer(file, counts, census, (StringBuffer*) &vm->methodNames);
}

void wrenSnapshotSave(WrenVM* vm, WrenCounts* counts, WrenCensus* census)
{
  FILE* file = fopen("bytecode", "wb");
  if (file == NULL) return;

  saveAllString   (file, counts, census);
  saveAllModule   (file, counts, census);
  saveAllFn       (file, counts, census);
  saveAllClosure  (file, counts, census);
  saveAllMap      (file, counts, census);
  saveAllClass    (file, counts, census);
  saveVM          (file, counts, census, vm);

  fclose(file);
}

// Snapshot restore ------------------------------------------------------------

#define FREAD fread   // TODO check returned value

#define FREAD_NUM(n)  FREAD(&n, sizeof(n), 1, file)

// TODO: don't dup from src/vm/wren_vm.c
static void performCount(WrenVM* vm)
{
#if WREN_SNAPSHOT
  WrenCounts counts = {0};

  wrenCountAllObj(vm, &counts);
#endif
}

#define DEFINE_restoreIdAsObj(type)                                            \
static Obj##type* restoreIdAsObj##type(WrenSnapshotContext* ctx)               \
{                                                                              \
  FILE* file = ctx->file;                                                      \
                                                                               \
  WrenCount id;                                                                \
  FREAD_NUM(id);                                                               \
                                                                               \
  VERBOSE printf("Obj" #type "#%lu", id);                                      \
                                                                               \
  if (id == 0) return NULL;                                                    \
                                                                               \
  Obj##type** all = ctx->census->all##type;                                    \
  if (all == NULL) { printf("\tNO [] => TO swizzle"); return NULL; } /*TODO*/  \
                                                                               \
  WrenCount nb = ctx->counts->nb##type;                                        \
  if (id > nb) { printf("\tOVERFLOW"); return NULL; } /*TODO*/                 \
                                                                               \
  Obj##type* obj = all[id - 1];                                                \
  if (obj == NULL) { printf("\tTO swizzle"); return NULL; } /*TODO*/           \
                                                                               \
  return obj;                                                                  \
}

// TODO append in a SwizzleBuffer, which references the type##Buffer

DEFINE_restoreIdAsObj(Class)
DEFINE_restoreIdAsObj(Fn)
DEFINE_restoreIdAsObj(Module)
DEFINE_restoreIdAsObj(String)
DEFINE_restoreIdAsObj(Closure)
DEFINE_restoreIdAsObj(Map)

static Value restoreValue(WrenSnapshotContext* ctx, WrenVM* vm)
{
  FILE* file = ctx->file;

  ObjOrValueType type;
  FREAD_NUM(type);

  VERBOSE printf("Value ");

  switch (type)
  {
    case OBJ_CLASS:
      ObjClass* class = restoreIdAsObjClass(ctx);
      return OBJ_VAL(class);

    //case OBJ_CLOSURE: break;
    //case OBJ_FIBER: break;
    case OBJ_FN:
      ObjFn* fn = restoreIdAsObjFn(ctx);
      return OBJ_VAL(fn);
    //case OBJ_FOREIGN: break;
    //case OBJ_INSTANCE: break;
    //case OBJ_LIST: break;
    //case OBJ_MAP: break;
    case OBJ_MODULE:
      ObjModule* module = restoreIdAsObjModule(ctx);
      return OBJ_VAL(module);
    //case OBJ_RANGE: break;
    //case OBJ_UPVALUE: break;
    case OBJ_STRING:
      ObjString* str = restoreIdAsObjString(ctx);
      return OBJ_VAL(str);

    //case ValueTypeCharFalse:  break;
    case ValueTypeCharNull:
      return NULL_VAL;

    case ValueTypeCharNum:
      double d;
      FREAD_NUM(d);
      Value v = NUM_VAL(d);
      VERBOSE wrenDumpValue_(stdout, v, true);
      return v;
    //case ValueTypeCharTrue:   break;
    //case ValueTypeCharNaN:    break;

    default:
      break;
  }

  // TODO
  VERBOSE printf("type=%u(%c) UNHANDLED!\n", type, type);

  ASSERT(false, "Snapshot restore fatal error.");

  return NULL_VAL; // TODO should disappear
}

static Method restoreMethod(WrenSnapshotContext* ctx)
{
  FILE* file = ctx->file;

  uint8_t type;
  FREAD_NUM(type);

  VERBOSE printf("%c ", type);

  Method m;

  switch (type)
  {
    case MethodTypeCharPrimitive:
      m.type = METHOD_PRIMITIVE;
      // TODO
      return m;

    case MethodTypeCharFunctionCall:
      m.type = METHOD_FUNCTION_CALL;
      // TODO
      return m;

    //TODO case MethodTypeCharForeign:

    case MethodTypeCharBlock:
      m.type = METHOD_BLOCK;
      ObjClosure* closure = restoreIdAsObjClosure(ctx);
      m.as.closure = closure;
      return m;

    case MethodTypeCharNone:
      m.type = METHOD_NONE;
      return m;
  }

  VERBOSE printf("Method type=%u(%c) UNHANDLED!\n", type, type);

  ASSERT(false, "Snapshot restore fatal error.");

  return m; // TODO should disappear
}

static ObjString* restoreObjString(WrenSnapshotContext* ctx, WrenVM* vm)
{
  FILE* file = ctx->file;

  uint32_t length;
  FREAD_NUM(length);

  char buf[256]; // TODO
  FREAD(buf, sizeof(char), length, file);

  Value v = wrenNewStringLength(vm, buf, length);

  return AS_STRING(v);
}

static void restoreStringBuffer(WrenSnapshotContext* ctx, WrenVM* vm, StringBuffer* buffer)
{
  FILE* file = ctx->file;
  int count;

  FREAD_NUM(count);

  VERBOSE printf("StringBuffer count = %u\n", count);

  // TODO validate count

  // TODO NICETOHAVE BufferEnsureSize()

  for (int i = 0; i < count; ++i)
  {
    VERBOSE printf("[%u]\t", i);
    ObjString* str = restoreIdAsObjString(ctx);
    wrenStringBufferWrite(vm, buffer, str);
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, OBJ_VAL(str), true);
    VERBOSE printf("\n");
  }
}

static void restoreValueBuffer(WrenSnapshotContext* ctx, WrenVM* vm, ValueBuffer* buffer)
{
  FILE* file = ctx->file;
  int count;

  FREAD_NUM(count);

  VERBOSE printf("ValueBuffer count = %u\n", count);

  // TODO validate count

  // TODO s/NICETOHAVE/MUSTHAVE/ BufferEnsureSize()

  for (int i = 0; i < count; ++i)
  {
    VERBOSE printf("[%u]\t", i);
    Value v = restoreValue(ctx, vm);
    wrenValueBufferWrite(vm, buffer, v);
    if (AS_OBJ(v) != NULL)
    {
      VERBOSE printf(" ");
      VERBOSE wrenDumpValue_(stdout, v, true);
    }
    VERBOSE printf("\n");
  }
}

static void restoreByteBuffer(WrenSnapshotContext* ctx, WrenVM* vm, ByteBuffer* buffer)
{
  FILE* file = ctx->file;
  int count;

  FREAD_NUM(count);

  VERBOSE printf("ByteBuffer count = %u\n", count);

  // TODO validate count

  // TODO s/NICETOHAVE/MUSTHAVE/ BufferEnsureSize()

  char buf[256]; // TODO
  FREAD(buf, sizeof(uint8_t), count, file);

  for (int i = 0; i < count; ++i)
  {
    // VERBOSE printf("[%u]\t", i);
    uint8_t byte = buf[i];
    wrenByteBufferWrite(vm, buffer, byte);
    // VERBOSE printf("\n");
  }
}

static void restoreMethodBuffer(WrenSnapshotContext* ctx, WrenVM* vm, MethodBuffer* buffer)
{
  FILE* file = ctx->file;
  int count;

  FREAD_NUM(count);

  VERBOSE printf("MethodBuffer count = %u\n", count);

  // TODO validate count

  // TODO s/NICETOHAVE/MUSTHAVE/ BufferEnsureSize()

  for (int i = 0; i < count; ++i)
  {
    VERBOSE printf("[%u]\t", i);
    Method m = restoreMethod(ctx);
    wrenMethodBufferWrite(vm, buffer, m);
    VERBOSE printf("\n");
  }
}

static ObjModule* restoreObjModule(WrenSnapshotContext* ctx, WrenVM* vm)
{
  FILE* file = ctx->file;

  ObjString* name = restoreIdAsObjString(ctx);

  if (name)
  {
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, OBJ_VAL(name), true);
  }
  VERBOSE printf("\n");

  ObjModule* module = wrenNewModule(vm, name);

  restoreStringBuffer(ctx, vm, (StringBuffer*) &module->variableNames);

  restoreValueBuffer(ctx, vm, &module->variables);

  return module;
}

static ObjFn* restoreObjFn(WrenSnapshotContext* ctx, WrenVM* vm)
{
  FILE* file = ctx->file;

  ObjModule* module = restoreIdAsObjModule(ctx);
  VERBOSE printf("\n");

  uint64_t lenName; // NOTE the type
  FREAD_NUM(lenName);

  char buf[256]; // TODO
  FREAD(buf, sizeof(char), lenName, file);

  uint8_t arity; // NOTE the type
  FREAD_NUM(arity);

  int maxSlots;
  FREAD_NUM(maxSlots);

  uint8_t numUpvalues; // NOTE the type
  FREAD_NUM(numUpvalues);

  ObjFn* fn = wrenNewFunction(vm, module, maxSlots);

  fn->arity = arity;
  fn->numUpvalues = numUpvalues;
  // VERBOSE if (numUpvalues != 0) printf("numUpvalues=%u", numUpvalues);

  wrenFunctionBindName(vm, fn, buf, (int) lenName); // NOTE the cast

  restoreValueBuffer(ctx, vm, &fn->constants);

  restoreByteBuffer(ctx, vm, &fn->code);
  // TODO validateBytecode(&fn->code);

  // TODO restoreIntBuffer(ctx, vm, &fn->debug->sourceLines);
  // wrenIntBufferFill(vm, &fn->debug->sourceLines, 0, fn->code.count);
  // But see blackenFn()

#if WREN_DEBUG_DUMP_COMPILED_CODE
  // TODO quick-n-dirty
  wrenStringBufferFill(vm, &vm->methodNames, ctx->census->allString[0], 256);
  VERBOSE wrenDumpCode(vm, fn);
#endif

  return fn;
}

static ObjClosure* restoreObjClosure(WrenSnapshotContext* ctx, WrenVM* vm)
{
  ObjFn* fn = restoreIdAsObjFn(ctx);

  VERBOSE printf("\n");

  ObjClosure* closure = wrenNewClosure(vm, fn);

  return closure;
}

static ObjMap* restoreObjMap(WrenSnapshotContext* ctx, WrenVM* vm)
{
  FILE* file = ctx->file;

  ObjMap* map = wrenNewMap(vm);

  uint32_t count;
  FREAD_NUM(count);

  VERBOSE printf("Map count = %u\n", count);

  // TODO resizeMap(vm, map, count); // but is static

  for (uint32_t i = 0; i < count; i++)
  {
    VERBOSE printf("Entry %u\n", i);

    VERBOSE printf(" k ");
    Value k = restoreValue(ctx, vm);
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, k, true);
    VERBOSE printf("\n");

    VERBOSE printf(" v ");
    Value v = restoreValue(ctx, vm);
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, v, true);
    VERBOSE printf("\n");

    wrenMapSet(vm, map, k, v);
  }

  return map;
}

static ObjClass* restoreObjClass(WrenSnapshotContext* ctx, WrenVM* vm)
{
  FILE* file = ctx->file;

  ObjString* name = restoreIdAsObjString(ctx);

  VERBOSE printf(" < ");

  ObjClass* super = restoreIdAsObjClass(ctx);

  VERBOSE printf("\n");

  Value attributes = restoreValue(ctx, vm);
  VERBOSE printf(" ");
  VERBOSE wrenDumpValue_(stdout, attributes, true);
  VERBOSE printf("\n");

  uint8_t numFields;
  FREAD_NUM(numFields);

  ObjClass* classObj = wrenNewSingleClass(vm, numFields, name);

  restoreMethodBuffer(ctx, vm, &classObj->methods);

  // TODO superclass: directly. Can't wrenBindSuperclass()
  // TODO attributes

  return classObj;
}

#define DEFINE_restoreAll(type    ,shunt) /*XXX*/                              \
static void restoreAll##type(WrenSnapshotContext* ctx, WrenVM* vm)             \
{                                                                              \
  FILE* file = ctx->file;                                                      \
  WrenCount nb;                                                                \
                                                                               \
  FREAD_NUM(nb);                                                               \
                                                                               \
  VERBOSE printf("\nNb = %lu\n", nb);                                          \
                                                                               \
  ctx->counts->nb##type = nb;                                                  \
                                                                               \
  ctx->census->all##type = ALLOCATE_ARRAY(vm, Obj##type*, nb);                 \
  memset(ctx->census->all##type, 0, sizeof(Obj##type*) * nb);                  \
                                                                               \
  for (WrenCount i = 0; i < nb; ++i)                                           \
  {                                                                            \
    VERBOSE printf("restoring Obj" #type "[%lu]\n", i);                        \
    Obj##type* obj = restoreObj##type(ctx, vm);                                \
    ctx->census->all##type[i] = obj;                                           \
    VERBOSE printf("\t\t\t---> ");                                             \
    VERBOSE wrenDumpValue_(stdout, OBJ_VAL(obj), true);                        \
    VERBOSE printf("\n");                                                      \
                                                                               \
    if (shunt) break; /*XXX*/                                                  \
  }                                                                            \
}

DEFINE_restoreAll(String        ,false)
DEFINE_restoreAll(Module        ,false)
DEFINE_restoreAll(Fn            ,false)
DEFINE_restoreAll(Closure       ,false)
DEFINE_restoreAll(Map           ,false)
DEFINE_restoreAll(Class         ,false)

#undef DEFINE_restoreAll

static void restoreVM(WrenSnapshotContext* ctx, WrenVM* vm)
{
  VERBOSE printf("\nrestoring VM\n");
#define RESTORE_CLASS(name)                                                   \
  vm->name##Class = restoreIdAsObjClass(ctx);                                 \
  VERBOSE printf("\t" #name "\n");

  RESTORE_CLASS(bool)
  RESTORE_CLASS(class)
  RESTORE_CLASS(fiber)
  RESTORE_CLASS(fn)
  RESTORE_CLASS(list)
  RESTORE_CLASS(map)
  RESTORE_CLASS(null)
  RESTORE_CLASS(num)
  RESTORE_CLASS(object)
  RESTORE_CLASS(range)
  RESTORE_CLASS(string)

#undef RESTORE_CLASS

  vm->modules = restoreIdAsObjMap(ctx);
  VERBOSE printf("\tmodules\n");

  vm->lastModule = restoreIdAsObjModule(ctx);
  VERBOSE printf("\tlastModule\n");

  restoreStringBuffer(ctx, vm, (StringBuffer*) &vm->methodNames);
}

static void printAllObj(WrenVM* vm) {
  printf("\n=== all Obj\n");
  WrenCount i = 0;
  for (Obj* obj = vm->first;
       obj != NULL;
       obj = obj->next, ++i)
  {
    printf("obj @ %lu =\t", i);
    wrenDumpValue_(stdout, OBJ_VAL(obj), true);
    printf("\n");
  }
}

  /*
  VERBOSE printf("=== allString\n");
  for (WrenCount i = 0; i < nb; ++i)
  {
    VERBOSE printf("%lu = ", i);
    wrenDumpValue_(stdout, OBJ_VAL(ctx->census->allString[i]), true);
    VERBOSE printf("\n");
  }
  */

void wrenSnapshotRestore(FILE* f, WrenVM* vm)
{
  // Expect no Obj yet.
  performCount(vm);

  // Set up an empty context.
  WrenCounts counts = {0};
  WrenCensus census = {0};
  WrenSnapshotContext ctx = { f, &counts, &census };

  // Restore all Obj.
  restoreAllString (&ctx, vm);
  restoreAllModule (&ctx, vm);
  restoreAllFn     (&ctx, vm);
  restoreAllClosure(&ctx, vm);
  restoreAllMap    (&ctx, vm);
  restoreAllClass  (&ctx, vm);
  restoreVM        (&ctx, vm);

  VERBOSE printAllObj(vm);

  /** /
  // Link all Obj together.
  swizzleObj();

    // TODO iterate all Obj, set its ->classObj according to its ->type:
      - ObjString   stringClass
      - ObjModule   NULL => nothing to do
      - ObjFn       fnClass
      - ObjClosure  fnClass
      - ObjMap      mapClass
      - ObjClass    classClass or a metaclass
  /**/

  // The census is no longer needed.
  wrenFreeCensus(vm, &census);

  performCount(vm);
  // TODO GC should be nop
  // performCount(vm);
}
