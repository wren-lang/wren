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

#if WREN_SNAPSHOT

// Record where to write a pointer which references a not-yet available object
// of [type] identified by [id].
// If [inValue], the [target] is [val], else [obj].
// NOTE: The location designated by [target] MUST NOT be reallocated elsewhere!
typedef struct {
  union {
    Obj** obj;
    Value* val;
  } target;
  WrenCount id;
  ObjType type;
  bool inValue;
} Swizzle;

DECLARE_BUFFER(Swizzle, Swizzle);
DEFINE_BUFFER(Swizzle, Swizzle)

struct sWrenSnapshotContext;

// Analogous to fread(), but abstracts away from FILE*.
typedef size_t (*WrenSnapshotReadFn)(void* ptr, size_t size, size_t nmemb, struct sWrenSnapshotContext* ctx);
// Analogous to fwrite(), but abstracts away from FILE*.
typedef size_t (*WrenSnapshotWriteFn)(const void* ptr, size_t size, size_t nmemb, struct sWrenSnapshotContext* ctx);

typedef struct sWrenSnapshotContext {
  union {
    WrenSnapshotReadFn read;
    WrenSnapshotWriteFn write;
  };
  void* priv;
  WrenCounts *counts;
  WrenCensus *census;
  SwizzleBuffer* swizzles;
} WrenSnapshotContext;

// Write into the [ctx], assuming its priv is a FILE*.
static size_t writeToFILE(const void* ptr, size_t size, size_t nmemb, WrenSnapshotContext* ctx)
{
  FILE* f = (FILE*)ctx->priv;

  //if (size == 1)
  return fwrite(ptr, size, nmemb, f);

  // TODO ensure nmemb is returned
  // TODO feof(), ferror()

  // TODO endianness when size > 1:
  //if (!size) return 0;
  //
  //ASSERT(nmemb == 1, "Won't handle endianness of several items.");
  //for (size_t i = 0; i < size; i++)
  //  fwrite(ptr + size - 1 - i, 1, 1, f);
  //return nmemb;
}

#define CHAR(oneCharStr) (ctx->write)(oneCharStr, sizeof(char), 1,               ctx)
#define STR_CONST(str)   (ctx->write)(str,        sizeof(char), sizeof(str) - 1, ctx)
#define STR(str)         (ctx->write)(str,        sizeof(char), strlen(str),     ctx)
#define NUM(n)           (ctx->write)(&n,         sizeof(n),    1,               ctx)
  // TODO check returned values

static const bool verbose = false;

#define VERBOSE    if (!verbose) {} else

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

// Helper method for wrenFindInCensus() on the census in the [ctx].
static WrenCount wrenFindInCtx(WrenSnapshotContext* ctx, Obj* needle)
{
  return wrenFindInCensus(ctx->counts, ctx->census, needle);
}

static void saveNum(WrenSnapshotContext* ctx, double num)
{
  const ObjOrValueType type = ValueTypeCharNum;
  NUM(type);
  NUM(num);                     // TODO portability?
}

static void saveObj(WrenSnapshotContext* ctx, Obj* obj)
{
  const ObjOrValueType type = obj->type;   // NOTE the cast
  WrenCount id = wrenFindInCtx(ctx, obj);

  NUM(type);
  NUM(id);
}

static void saveValue(WrenSnapshotContext* ctx, Value v)
{
#if WREN_NAN_TAGGING
  if (IS_NUM(v))
  {
    saveNum(ctx, AS_NUM(v));
  }
  else if (IS_OBJ(v))
  {
    saveObj(ctx, AS_OBJ(v));
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
    case VAL_NUM:   saveNum(ctx, AS_NUM(v)); break;
    case VAL_OBJ:   saveObj(ctx, AS_OBJ(v)); break;
    case VAL_UNDEFINED: UNREACHABLE();
  }
#endif
}

#define SAVE_BUFFER(name, type)                                                \
static void save##name##Buffer(WrenSnapshotContext* ctx, name##Buffer* buffer) \
{                                                                              \
  const int count = buffer->count;                                             \
  type* data = buffer->data;                                                   \
                                                                               \
  NUM(count);                                                                  \
  VERBOSE CHAR("{");                                                           \
  for (int i = 0; i < count; ++i)                                              \
  {                                                                            \
    type item = data[i];                                                       \
    VERBOSE if (i) CHAR(",");                                                  \
    save##name(ctx, item);                                                     \
  }                                                                            \
  VERBOSE CHAR("}");                                                           \
}
SAVE_BUFFER(Value, Value)

static void saveString(WrenSnapshotContext* ctx, ObjString* str)
{
  WrenCount id = wrenFindInCtx(ctx, (Obj*)str);
  NUM(id);
}
SAVE_BUFFER(String, ObjString*)

// TODO move in a .h file
size_t findPrimitiveInCensus(Primitive prim);
Primitive getPrimitive(size_t index);

enum {
  MethodTypeCharPrimitive     = 'P',
  MethodTypeCharFunctionCall  = 'C',  // call
  MethodTypeCharForeign       = 'F',
  MethodTypeCharBlock         = 'M',  // method
  MethodTypeCharNone          = ' ',
};

static void saveMethod(WrenSnapshotContext* ctx, Method m)
{
  uint8_t type;             // NOTE the type
  switch (m.type)
  {
    case METHOD_PRIMITIVE:
    case METHOD_FUNCTION_CALL:
    {
      type = m.type == METHOD_FUNCTION_CALL
        ? MethodTypeCharFunctionCall
        : MethodTypeCharPrimitive;
      NUM(type);

      // NOTE the type and 0-based
      const uint8_t index = findPrimitiveInCensus(m.as.primitive);
      NUM(index);
      break;
    }

    case METHOD_FOREIGN:
      type = MethodTypeCharForeign;
      NUM(type);
      // TODO id for a WrenForeignMethodFn
      break;

    case METHOD_BLOCK:
    {
      type = MethodTypeCharBlock;
      NUM(type);

      WrenCount id = wrenFindInCtx(ctx, (Obj*)m.as.closure);
      NUM(id);
      break;
    }

    case METHOD_NONE:
      type = MethodTypeCharNone;
      NUM(type);
      break;
  }
}
SAVE_BUFFER(Method, Method)

static void saveInt(WrenSnapshotContext* ctx, int i)
{
  NUM(i);
}
SAVE_BUFFER(Int, int)

static void saveByteBuffer(WrenSnapshotContext* ctx, ByteBuffer* buffer)
{
  const int count = buffer->count;
  const uint8_t* data = buffer->data;

  NUM(count);
  VERBOSE CHAR("{");
  (ctx->write)(data, sizeof(uint8_t), count, ctx);
  VERBOSE CHAR("}");
}

static void saveSymbolTable(WrenSnapshotContext* ctx, SymbolTable* symtab)
{
  saveStringBuffer(ctx, (StringBuffer*)symtab);
}

static void saveObjString(WrenSnapshotContext* ctx, ObjString* str)
{
  uint32_t length = str->length;
  NUM(length);
  VERBOSE CHAR("\"");
  (ctx->write)(str->value, sizeof(char), length, ctx);
  VERBOSE CHAR("\"");
}

static void saveObjModule(WrenSnapshotContext* ctx, ObjModule* module)
{
  // The core module has no name, hence id_name will be 0.
  ObjString* name = module->name;
  WrenCount id_name = wrenFindInCtx(ctx, (Obj*)name);
  NUM(id_name);

  saveSymbolTable(ctx, &module->variableNames);
  saveValueBuffer(ctx, &module->variables);
}

static void saveObjFn(WrenSnapshotContext* ctx, ObjFn* fn)
{
  const ObjModule* module = fn->module;
  WrenCount id_module = wrenFindInCtx(ctx, (Obj*)module);
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

  VERBOSE CHAR("K");
  saveValueBuffer(ctx, &fn->constants);

  VERBOSE CHAR("C");
  saveByteBuffer(ctx, &fn->code);

  VERBOSE CHAR("D");
  saveIntBuffer(ctx, &fn->debug->sourceLines);
}

static void saveObjClosure(WrenSnapshotContext* ctx, ObjClosure* closure)
{
  const ObjFn* fn = closure->fn;
  WrenCount id_fn = wrenFindInCtx(ctx, (Obj*)fn);
  NUM(id_fn);

  // TODO upvalues, fn->numUpvalues
}

static void saveObjMap(WrenSnapshotContext* ctx, ObjMap* map)
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

      saveValue(ctx, entry->key);
      VERBOSE CHAR("=");
      VERBOSE CHAR(">");
      saveValue(ctx, entry->value);
    }
    VERBOSE CHAR("}");
  }
}

static void saveObjClass(WrenSnapshotContext* ctx, ObjClass* classObj)
{
  ObjString* name = classObj->name;
  WrenCount id_name = wrenFindInCtx(ctx, (Obj*)name);
  NUM(id_name);

  VERBOSE CHAR("F");
  const uint8_t numFields = classObj->numFields;    // NOTE the type; see MAX_FIELDS
  NUM(numFields);

  VERBOSE CHAR("<");
  ObjClass* super = classObj->superclass;
  WrenCount id_super = wrenFindInCtx(ctx, (Obj*)super);
  NUM(id_super);

  VERBOSE CHAR("m");
  ObjClass* meta = classObj->obj.classObj;
  WrenCount id_meta = wrenFindInCtx(ctx, (Obj*)meta);
  NUM(id_meta);

  VERBOSE CHAR("A");
  saveValue(ctx, classObj->attributes);

  VERBOSE CHAR("M");
  saveMethodBuffer(ctx, &classObj->methods);
}

#define SAVE_ALL(type)                                                         \
static void saveAll##type(WrenSnapshotContext* ctx)                            \
{                                                                              \
  static const char strType[] = "Obj" #type;                                   \
                                                                               \
  const WrenCount nb = ctx->counts->nb##type;                                  \
  Obj##type** all    = ctx->census->all##type;                                 \
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
    saveObj##type(ctx, obj);                                                   \
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

static void saveMethodNames(WrenSnapshotContext* ctx, WrenVM* vm)
{
  VERBOSE CHAR("M");
  saveSymbolTable(ctx, &vm->methodNames);
  VERBOSE CHAR("\n");
}

static void saveVM(WrenSnapshotContext* ctx, WrenVM* vm, ObjClosure* entrypoint)
{
  VERBOSE CHAR("V");
  VERBOSE CHAR("M");
  VERBOSE CHAR("\n");

#define SAVE_CLASS(verboseCharStr, name)                                       \
  VERBOSE CHAR(verboseCharStr);                                                \
  ObjClass* name##Class = vm->name##Class;                                     \
  const WrenCount id_##name##Class = wrenFindInCtx(ctx, (Obj*)name##Class);    \
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
  const WrenCount id_modules = wrenFindInCtx(ctx, (Obj*)vm->modules);
  NUM(id_modules);
  VERBOSE CHAR("\n");

  VERBOSE CHAR("I");  // import
  const WrenCount id_lastModule = wrenFindInCtx(ctx, (Obj*)vm->lastModule);
  NUM(id_lastModule);
  VERBOSE CHAR("\n");

  // methodNames was saved before.

  VERBOSE CHAR("@");
  const WrenCount id_entrypoint = wrenFindInCtx(ctx, (Obj*)entrypoint);
  NUM(id_entrypoint);
  VERBOSE CHAR("\n");
}

void wrenSnapshotSave(WrenVM* vm, WrenCounts* counts, WrenCensus* census, ObjClosure* entrypoint)
{
  FILE* file = fopen("bytecode", "wb");
  if (file == NULL) return;

  WrenSnapshotContext ctx = {
    { .write = writeToFILE }, file, counts, census, NULL
  };

  saveAllString   (&ctx);
  saveMethodNames (&ctx, vm);
  saveAllModule   (&ctx);
  saveAllFn       (&ctx);
  saveAllClosure  (&ctx);
  saveAllMap      (&ctx);
  saveAllClass    (&ctx);
  saveVM          (&ctx, vm, entrypoint);

  fclose(file);
}

// Snapshot restore ------------------------------------------------------------

// Read from the [ctx], assuming its priv is a FILE*.
static size_t readFromFILE(void* ptr, size_t size, size_t nmemb, WrenSnapshotContext* ctx)
{
  FILE* f = (FILE*)ctx->priv;
  return fread(ptr, size, nmemb, f);
  // TODO ensure nmemb is returned
  // TODO feof(), ferror()
  // TODO endianness when size > 1
}

#define FREAD_NUM(n)  (ctx->read)(&n, sizeof(n), 1, ctx)

// TODO: don't dup from src/vm/wren_vm.c
static void performCount(WrenVM* vm)
{
#if WREN_SNAPSHOT
  WrenCounts counts = {0};

  wrenCountAllObj(vm, &counts);
#endif
}

#define DEFINE_restoreIdAsObj(type)                                            \
static Obj##type* getObj##type(WrenSnapshotContext* ctx, WrenCount id)         \
{                                                                              \
  Obj##type** all = ctx->census->all##type;                                    \
  if (all != NULL) {                                                           \
    WrenCount nb = ctx->counts->nb##type;                                      \
    if (id > nb) { VERBOSE printf("\tOVERFLOW"); return NULL; } /*TODO*/       \
                                                                               \
    Obj##type* obj = all[id - 1];                                              \
    if (obj != NULL) return obj;                                               \
  }                                                                            \
  return NULL;                                                                 \
}                                                                              \
                                                                               \
static Obj##type* restoreIdAsObj##type(WrenSnapshotContext* ctx, WrenCount* p) \
{                                                                              \
  WrenCount id;                                                                \
  FREAD_NUM(id);                                                               \
                                                                               \
  VERBOSE printf("Obj" #type "#%u", id);                                       \
                                                                               \
  if (id == 0) return NULL;                                                    \
                                                                               \
  Obj##type* obj = getObj##type(ctx, id);                                      \
  if (obj != NULL) return obj;                                                 \
                                                                               \
  if (p == NULL)                                                               \
  {                                                                            \
    VERBOSE printf("\n");                                                      \
    ASSERT(false, "Caller expects no swizzles.");                              \
  }                                                                            \
                                                                               \
  *p = id;                                                                     \
  return NULL;                                                                 \
}

DEFINE_restoreIdAsObj(Class)
DEFINE_restoreIdAsObj(Fn)
DEFINE_restoreIdAsObj(Module)
DEFINE_restoreIdAsObj(String)
DEFINE_restoreIdAsObj(Closure)
DEFINE_restoreIdAsObj(Map)

#undef DEFINE_restoreIdAsObj

static Primitive restoreIndexAsPrimitive(WrenSnapshotContext* ctx)
{
  uint8_t index;
  FREAD_NUM(index);

  VERBOSE printf("primitive[%u]", index);

  const Primitive prim = getPrimitive(index);

  if (prim == NULL) { VERBOSE printf("\tOVERFLOW"); return NULL; } /*TODO*/

  return prim;
}

static Value restoreValue(WrenSnapshotContext* ctx, Swizzle* swizzle)
{
  ObjOrValueType type;
  FREAD_NUM(type);

  VERBOSE printf("Value ");

  #define RETURN_ID_AS_OBJ(t)                                                  \
    do {                                                                       \
      Obj##t* obj = restoreIdAsObj##t(ctx, NULL);                              \
      return OBJ_VAL(obj);                                                     \
    } while (false)

  #define RETURN_ID_AS_MAYBE_SWIZZLED_OBJ(t)                                   \
    do {                                                                       \
      Obj##t* obj = restoreIdAsObj##t(ctx, &swizzle->id);                      \
      if (obj != NULL) return OBJ_VAL(obj);                                    \
      swizzle->type = type;                                                    \
      return UNDEFINED_VAL;                                                    \
    } while (false)

  switch (type)
  {
    case OBJ_CLASS:       RETURN_ID_AS_MAYBE_SWIZZLED_OBJ(Class);
    //case OBJ_CLOSURE: break;
    //case OBJ_FIBER: break;
    case OBJ_FN:          RETURN_ID_AS_MAYBE_SWIZZLED_OBJ(Fn);
    //case OBJ_FOREIGN: break;
    //case OBJ_INSTANCE: break;
    //case OBJ_LIST: break;
    //case OBJ_MAP: break;
    case OBJ_MODULE:      RETURN_ID_AS_OBJ(Module);
    //case OBJ_RANGE: break;
    //case OBJ_UPVALUE: break;
    case OBJ_STRING:      RETURN_ID_AS_OBJ(String);

    case ValueTypeCharFalse:  return FALSE_VAL;
    case ValueTypeCharNull:   return NULL_VAL;
    case ValueTypeCharTrue:   return TRUE_VAL;

    case ValueTypeCharNum:
    {
      double d;
      FREAD_NUM(d);
      Value v = NUM_VAL(d);
      VERBOSE wrenDumpValue_(stdout, v, true);
      return v;
    }

    //case ValueTypeCharNaN:    break;

    default:
      break;
  }

  #undef RETURN_ID_AS_OBJ
  #undef RETURN_ID_AS_MAYBE_SWIZZLED_OBJ

  // TODO
  VERBOSE printf("type=%u(%c) UNHANDLED!\n", type, type);

  ASSERT(false, "Snapshot restore fatal error.");

  return NULL_VAL; // TODO should disappear
}

static Method restoreMethod(WrenSnapshotContext* ctx)
{
  uint8_t type;
  FREAD_NUM(type);

  VERBOSE printf("%c ", type);

  Method m;

  switch (type)
  {
    case MethodTypeCharPrimitive:
    case MethodTypeCharFunctionCall:
    {
      m.type = type == MethodTypeCharFunctionCall
        ? METHOD_FUNCTION_CALL
        : METHOD_PRIMITIVE;

      const Primitive prim = restoreIndexAsPrimitive(ctx);
      m.as.primitive = prim;
      return m;
    }

    //TODO case MethodTypeCharForeign:

    case MethodTypeCharBlock:
      m.type = METHOD_BLOCK;
      ObjClosure* closure = restoreIdAsObjClosure(ctx, NULL);
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
  uint32_t length;
  FREAD_NUM(length);

  char buf[256]; // TODO
  ASSERT(length <= sizeof(buf), "Buffer too small.");
  (ctx->read)(buf, sizeof(char), length, ctx);

  Value v = wrenNewStringLength(vm, buf, length);
  // TODO allocateString(); (ctx->read)(); hashString(); // but they're static

  return AS_STRING(v);
}

#define RESTORE_BUFFER(name, type)                                             \
static void restore##name##Buffer(WrenSnapshotContext* ctx, WrenVM* vm, name##Buffer* buffer) \
{                                                                              \
  int count;                                                                   \
  FREAD_NUM(count);                                                            \
                                                                               \
  VERBOSE printf(#name "Buffer count = %u\n", count);                          \
                                                                               \
  /* TODO validate count */                                                    \
                                                                               \
  wren##name##BufferEnsure(vm, buffer, count);                                 \
                                                                               \
  for (int i = 0; i < count; ++i)                                              \
  {                                                                            \
    VERBOSE printf("[%u]\t", i);                                               \
    type item = restore##name(ctx);                                            \
    wren##name##BufferWrite(vm, buffer, item);                                 \
    VERBOSE printf("\n");                                                      \
  }                                                                            \
}

static ObjString* restoreString(WrenSnapshotContext* ctx)
{
  ObjString* str = restoreIdAsObjString(ctx, NULL);
  VERBOSE printf(" ");
  VERBOSE wrenDumpValue_(stdout, OBJ_VAL(str), true);
  return str;
}
RESTORE_BUFFER(String, ObjString*)

static int restoreInt(WrenSnapshotContext* ctx)
{
  int i;
  FREAD_NUM(i);
  VERBOSE printf("%d", i);
  return i;
}
RESTORE_BUFFER(Int, int)

// Returns true iff there was any swizzle found.
static bool restoreValueBuffer(WrenSnapshotContext* ctx, WrenVM* vm, ValueBuffer* buffer)
{
  int count;
  FREAD_NUM(count);

  VERBOSE printf("ValueBuffer count = %u\n", count);

  // TODO validate count

  wrenValueBufferEnsure(vm, buffer, count);

  bool hadSwizzle = false;

  for (int i = 0; i < count; ++i)
  {
    VERBOSE printf("[%u]\t", i);
    Swizzle swizzle;
    Value v = restoreValue(ctx, &swizzle);
    if (IS_UNDEFINED(v))
    {
      VERBOSE printf(" TO swizzle");
      swizzle.inValue = true;
      swizzle.target.val = &buffer->data[buffer->count];
      wrenSwizzleBufferWrite(vm, ctx->swizzles, swizzle);
      hadSwizzle = true;
    } else {
      VERBOSE printf(" ");
      VERBOSE wrenDumpValue_(stdout, v, true);
    }
    wrenValueBufferWrite(vm, buffer, v);
    VERBOSE printf("\n");
  }

  return hadSwizzle;
}

static void restoreByteBuffer(WrenSnapshotContext* ctx, WrenVM* vm, ByteBuffer* buffer)
{
  int count;
  FREAD_NUM(count);

  VERBOSE printf("ByteBuffer count = %u\n", count);

  // TODO validate count

  wrenByteBufferEnsure(vm, buffer, count);

  (ctx->read)(buffer->data, sizeof(uint8_t), count, ctx);
}

RESTORE_BUFFER(Method, Method)

static void restoreSymbolTable(WrenSnapshotContext* ctx, WrenVM* vm, SymbolTable* symtab)
{
  restoreStringBuffer(ctx, vm, (StringBuffer*)symtab);
}

static ObjModule* restoreObjModule(WrenSnapshotContext* ctx, WrenVM* vm)
{
  ObjString* name = restoreIdAsObjString(ctx, NULL);

  if (name)
  {
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, OBJ_VAL(name), true);
  }
  VERBOSE printf("\n");

  ObjModule* module = wrenNewModule(vm, name);

  restoreSymbolTable(ctx, vm, &module->variableNames);

  restoreValueBuffer(ctx, vm, &module->variables);

  return module;
}

static ObjFn* restoreObjFn(WrenSnapshotContext* ctx, WrenVM* vm)
{
  ObjModule* module = restoreIdAsObjModule(ctx, NULL);
  VERBOSE printf("\n");

  uint64_t lenName; // NOTE the type
  FREAD_NUM(lenName);

  char buf[256]; // TODO
  ASSERT(lenName <= sizeof(buf), "Buffer too small.");
  (ctx->read)(buf, sizeof(char), lenName, ctx);

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

#if WREN_DEBUG_DUMP_COMPILED_CODE
  const bool hadSwizzle =
#endif
  restoreValueBuffer(ctx, vm, &fn->constants);

  restoreByteBuffer(ctx, vm, &fn->code);
  // TODO validateBytecode(&fn->code);

  restoreIntBuffer(ctx, vm, &fn->debug->sourceLines);
  // TODO see blackenFn()

#if WREN_DEBUG_DUMP_COMPILED_CODE
  if (hadSwizzle)
  {
    VERBOSE printf("// wrenDumpCode() would ASSERT() on unexpected swizzle in constant\n");
  }
  else
  {
    VERBOSE wrenDumpCode(vm, fn);
  }
#endif

  return fn;
}

static ObjClosure* restoreObjClosure(WrenSnapshotContext* ctx, WrenVM* vm)
{
  ObjFn* fn = restoreIdAsObjFn(ctx, NULL);

  VERBOSE printf("\n");

  ObjClosure* closure = wrenNewClosure(vm, fn);

  return closure;
}

static ObjMap* restoreObjMap(WrenSnapshotContext* ctx, WrenVM* vm)
{
  ObjMap* map = wrenNewMap(vm);

  uint32_t count;
  FREAD_NUM(count);

  VERBOSE printf("Map count = %u\n", count);

  // TODO resizeMap(vm, map, count); // but is static

  for (uint32_t i = 0; i < count; i++)
  {
    VERBOSE printf("Entry %u\n", i);

    VERBOSE printf(" k ");
    Value k = restoreValue(ctx, NULL);
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, k, true);
    VERBOSE printf("\n");

    VERBOSE printf(" v ");
    Value v = restoreValue(ctx, NULL);
    VERBOSE printf(" ");
    VERBOSE wrenDumpValue_(stdout, v, true);
    VERBOSE printf("\n");

    wrenMapSet(vm, map, k, v);
  }

  return map;
}

static ObjClass* restoreObjClass(WrenSnapshotContext* ctx, WrenVM* vm)
{
  ObjString* name = restoreIdAsObjString(ctx, NULL);

  uint8_t numFields;
  FREAD_NUM(numFields);

  VERBOSE printf(" fields=%u", numFields);

  ObjClass* classObj = wrenNewSingleClass(vm, numFields, name);

  VERBOSE printf(" < ");

  ObjClass* super = restoreIdAsObjClass(ctx, NULL);
  // NULL is expected for Object; it's never ambiguous as there are no swizzles here.
  classObj->superclass = super;

  VERBOSE printf(" m ");

  Swizzle swizzle;
  ObjClass* meta = restoreIdAsObjClass(ctx, &swizzle.id);
  if (meta != NULL)
  {
    classObj->obj.classObj = meta;
  } else {
    VERBOSE printf(" TO swizzle");
    swizzle.type = OBJ_CLASS;
    swizzle.inValue = false;
    swizzle.target.obj = (Obj**)&classObj->obj.classObj;
    wrenSwizzleBufferWrite(vm, ctx->swizzles, swizzle);
  }

  VERBOSE printf("\n");

  VERBOSE printf("attr\t");
  classObj->attributes = restoreValue(ctx, NULL);
  VERBOSE wrenDumpValue_(stdout, classObj->attributes, true);
  VERBOSE printf("\n");

  restoreMethodBuffer(ctx, vm, &classObj->methods);

  return classObj;
}

#define DEFINE_restoreAll(type    ,shunt) /*XXX*/                              \
static void restoreAll##type(WrenSnapshotContext* ctx, WrenVM* vm)             \
{                                                                              \
  WrenCount nb;                                                                \
  FREAD_NUM(nb);                                                               \
                                                                               \
  VERBOSE printf("\nNb = %u\n", nb);                                           \
                                                                               \
  ctx->counts->nb##type = nb;                                                  \
                                                                               \
  ctx->census->all##type = ALLOCATE_ARRAY(vm, Obj##type*, nb);                 \
  memset(ctx->census->all##type, 0, sizeof(Obj##type*) * nb);                  \
                                                                               \
  for (WrenCount i = 0; i < nb; ++i)                                           \
  {                                                                            \
    VERBOSE printf("restoring Obj" #type "[%u]\n", i);                         \
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

static void restoreMethodNames(WrenSnapshotContext* ctx, WrenVM* vm)
{
  restoreSymbolTable(ctx, vm, &vm->methodNames);
}

static ObjClosure* restoreVM(WrenSnapshotContext* ctx, WrenVM* vm)
{
  VERBOSE printf("\nrestoring VM\n");
#define RESTORE_CLASS(name)                                                   \
  vm->name##Class = restoreIdAsObjClass(ctx, NULL);                           \
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

  vm->modules = restoreIdAsObjMap(ctx, NULL);
  VERBOSE printf("\tmodules\n");

  vm->lastModule = restoreIdAsObjModule(ctx, NULL);
  VERBOSE printf("\tlastModule\n");

  // methodNames was restored before.

  ObjClosure* entrypoint = restoreIdAsObjClosure(ctx, NULL);
  VERBOSE printf("\tentrypoint\n");

  return entrypoint;
}

static void printAllObj(WrenVM* vm) {
  printf("\n=== all Obj\n");
  WrenCount i = 0;
  for (Obj* obj = vm->first;
       obj != NULL;
       obj = obj->next, ++i)
  {
    printf("obj @ %u =\t", i);
    printf("class=%p\t", obj->classObj);
    wrenDumpValue_(stdout, OBJ_VAL(obj), true);
    printf("\n");
  }
}

static void swizzlePointers(WrenSnapshotContext* ctx)
{
  const SwizzleBuffer* buffer = ctx->swizzles;

  for (int i = 0; i < buffer->count; ++i)
  {
    const Swizzle* swiz = &buffer->data[i];

    VERBOSE printf("swizzle %s %p with %u#%u\n",
      swiz->inValue ? "Value" : "Obj  ",
      swiz->target.val, swiz->type, swiz->id);

    Obj* obj;
    switch (swiz->type)
    {
      case OBJ_CLASS: obj = (Obj*)getObjClass(ctx, swiz->id); break;
      case OBJ_FN:    obj = (Obj*)getObjFn   (ctx, swiz->id); break;
      default:
        ASSERT(false, "UNHANDLED ObjType.");
        obj = NULL;
        break;
    }
    ASSERT(obj != NULL, "Swizzle found no substitute.");

    if (swiz->inValue)
    {
      Value v = *swiz->target.val;
      ASSERT(IS_UNDEFINED(v), "Will swizzle only an UNDEFINED_VAL.");
      *swiz->target.val = OBJ_VAL(obj);
    } else {
      Obj* o = *swiz->target.obj;
      ASSERT(o == NULL, "Will swizzle only a NULL Obj*.");
      *swiz->target.obj = obj;
    }
  }
}

static void assignClasses(WrenVM* vm)
{
  VERBOSE printf("\n=== assign Classes\n");
  for (Obj* obj = vm->first;
       obj != NULL;
       obj = obj->next)
    switch (obj->type)
    {
      case OBJ_CLASS: break;  // Its metaclass is already restored.
      case OBJ_CLOSURE:
      case OBJ_FN:
        obj->classObj = vm->fnClass;
        break;
      case OBJ_MAP:    obj->classObj = vm->mapClass; break;
      case OBJ_MODULE: break; // Modules are not first-class objects in Wren.
      case OBJ_STRING: obj->classObj = vm->stringClass; break;
      default:
        ASSERT(false, "UNHANDLED ObjType.");
        break;
    }
}

static void slurpFile(FILE* f, WrenVM* vm, ByteBuffer* buf)
{
  for (;;)
  {
    uint8_t byte;
    const size_t nb = fread(&byte, sizeof(byte), 1, f);
    if (nb != 1)
    {
      if (feof (f)) break;
      printf("slurp fread()=%zu\n", nb);
      ASSERT(!ferror(f), "ferror.");
      ASSERT(false, "Unhandled I/O error.");
    }
    wrenByteBufferWrite(vm, buf, byte);
  }

  if (false) printf("slurped %d\n", buf->count);
}

typedef struct {
  const uint8_t* bytes;
  const size_t count;
  size_t offset;
} WrenStreamFromROBytes;

// Read from the [ctx], assuming its priv is a WrenStreamFromROBytes*.
static size_t readFromROBytes(void* ptr, size_t size, size_t nmemb, WrenSnapshotContext* ctx)
{
  uint8_t* p = (uint8_t*)ptr;
  WrenStreamFromROBytes* stream = (WrenStreamFromROBytes*)ctx->priv;

  for (size_t n = 0; n < nmemb; ++n)
  {
    for (size_t i = 0; i < size; ++i)
    {
      if (stream->offset >= stream->count) return n;

      const uint8_t byte = stream->bytes[stream->offset++];
      *p++ = byte;    // TODO endianness when size > 1
    }
  }

  return nmemb;
}

// #include "bytecode-mandelbroU.wrenb.c"

ObjClosure* wrenSnapshotRestore(FILE* f, WrenVM* vm)
{
  // Expect no Obj yet.
  performCount(vm);

  // Set up an empty context.

  WrenCounts counts = {0};
  WrenCensus census = {0};

  SwizzleBuffer swizzles;
  wrenSwizzleBufferInit(&swizzles);

  /*
  WrenStreamFromROBytes streamFromROBytes = {
    bytecode_mandelbroU_wrenb,
    bytecode_mandelbroU_wrenb_len,
    0
  };
  */

  WrenSnapshotContext ctx =
    { { .read = readFromFILE }, f, &counts, &census, &swizzles }

    /*
    { { .read = readFromROBytes }, &streamFromROBytes, &counts, &census, &swizzles }
    */
  ;

  // Restore all Obj.
  restoreAllString  (&ctx, vm);
  restoreMethodNames(&ctx, vm);
  restoreAllModule  (&ctx, vm);
  restoreAllFn      (&ctx, vm);
  restoreAllClosure (&ctx, vm);
  restoreAllMap     (&ctx, vm);
  restoreAllClass   (&ctx, vm);

  ObjClosure* entrypoint = restoreVM(&ctx, vm);

  VERBOSE printAllObj(vm);

  // Swizzle references into real pointers.
  swizzlePointers(&ctx);

  wrenSwizzleBufferClear(vm, &swizzles);

  // Assign class to each object.
  assignClasses(vm);

  VERBOSE printAllObj(vm);

  // The census is no longer needed.
  wrenFreeCensus(vm, &census);

  performCount(vm);
  // TODO GC should be nop
  // performCount(vm);

  return entrypoint;
}

#endif // WREN_SNAPSHOT
