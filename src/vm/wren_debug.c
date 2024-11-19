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
    int line = fn->debug->sourceLines.data[frame->ip - fn->code.data - 1];
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

  int line = fn->debug->sourceLines.data[i];
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

static void wrenSaveCode(WrenVM* vm, ObjFn* fn)
{
  static const char strFn[] = "ObjFn";

  FILE* file = vm->bytecodeFile;

  const int nbCode = fn->code.count;
  const int nbConst = fn->constants.count;

  // const ObjModule* module = fn->module;

#define CHAR(oneCharStr) fwrite(oneCharStr, sizeof(char),    1, file)
#define STR_CONST(str)   fwrite(str,        sizeof(str) - 1, 1, file)
#define STR(str)         fwrite(str,        strlen(str),     1, file)
#define NUM(n)           fwrite(&n,         sizeof(n),       1, file)


  // TODO check returned values
  STR_CONST(strFn);
  CHAR(":");
  //// not yet available
  // if (module->name) {
  //   wrenDumpValue_(file, OBJ_VAL(module->name), false);
  //   CHAR(":");
  //   CHAR(":");
  // }
  STR(fn->debug->name);
  // TODO fn->arity
  CHAR("\n");

  CHAR("\t");
  NUM(nbCode);
  CHAR("O");
  CHAR("{");
  fwrite(fn->code.data, sizeof(uint8_t), nbCode, file);
  CHAR("}");
  CHAR("\n");

  if (nbConst) {
    CHAR("\t");
    NUM(nbConst);
    CHAR("C");
    CHAR("{");
    for (int i = 0; i < nbConst; ++i) {
      if (i) CHAR(",");
      wrenDumpValue_(file, fn->constants.data[i], true);
    }
    CHAR("}");
    CHAR("\n");
  }

// #undef CHAR
// #undef STR
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

#if WREN_SNAPSHOT
  wrenSaveCode(vm, fn);
#endif
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

static const bool verbose = false;

#define VERBOSE    if (verbose)

// A char to discriminate the type. This augments the list in ObjType.
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
    const uint8_t type = ValueTypeCharNum; // NOTE the type
    double d = AS_NUM(v);
    NUM(type);
    NUM(d);                       // TODO portability?
  }
  else if (IS_OBJ(v))
  {
    Obj* obj = AS_OBJ(v);
    const uint8_t type = obj->type;   // NOTE the type and the cast
    WrenCount id = wrenFindInCensus(counts, census, obj);

    NUM(type);
    NUM(id);
  }
  else
  {
    uint8_t type;             // NOTE the type
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
  uint8_t type;             // NOTE the type
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
  NUM(fn->maxSlots);

  VERBOSE CHAR("U");
  const uint8_t numUpvalues = fn->numUpvalues;    // NOTE the type; see MAX_UPVALUES
  NUM(numUpvalues);

  VERBOSE CHAR("C");
  saveValueBuffer(file, counts, census, &fn->constants);

  VERBOSE CHAR("B");
  saveByteBuffer(file, counts, census, &fn->code);

  // TODO FnDebug* debug->sourceLines
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
  saveAllString   (vm->bytecodeFile, counts, census);
  saveAllModule   (vm->bytecodeFile, counts, census);
  saveAllFn       (vm->bytecodeFile, counts, census);
  saveAllClosure  (vm->bytecodeFile, counts, census);
  saveAllMap      (vm->bytecodeFile, counts, census);
  saveAllClass    (vm->bytecodeFile, counts, census);
  saveVM          (vm->bytecodeFile, counts, census, vm);
}
