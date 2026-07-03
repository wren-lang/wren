#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "wren.h"
#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_math.h"
#include "wren_utils.h"
#include "wren_value.h"
#include "wren_vm.h"

// The serializer exports a compiled module into a version-locked artifact.
//
// It reuses the normal VM + compiler path: a fresh WrenVM is created, a module
// is initialized with the core module's variables, source is compiled, then the
// resulting ObjFn tree is walked and written to a byte buffer. No special IR
// is introduced.

// Header flag bits.
#define HEADER_FLAG_DEBUG_INFO 0x01

// Constant-table tag bytes.
typedef enum
{
  CONSTANT_NULL,
  CONSTANT_FALSE,
  CONSTANT_TRUE,
  CONSTANT_NUM,
  CONSTANT_STRING,
  CONSTANT_FN
} ConstantTag;

typedef struct
{
  WrenVM* vm;
  ByteBuffer buffer;
  bool debugInfo;
  bool ok;
} Serializer;

static void writeByte(Serializer* serializer, uint8_t byte)
{
  wrenByteBufferWrite(serializer->vm, &serializer->buffer, byte);
}

static void writeUint16(Serializer* serializer, uint16_t value)
{
  writeByte(serializer, (uint8_t)(value >> 8));
  writeByte(serializer, (uint8_t)(value));
}

static void writeUint32(Serializer* serializer, uint32_t value)
{
  writeByte(serializer, (uint8_t)(value >> 24));
  writeByte(serializer, (uint8_t)(value >> 16));
  writeByte(serializer, (uint8_t)(value >> 8));
  writeByte(serializer, (uint8_t)(value));
}

static void writeDouble(Serializer* serializer, double value)
{
  uint64_t bits = wrenDoubleToBits(value);
  writeByte(serializer, (uint8_t)(bits >> 56));
  writeByte(serializer, (uint8_t)(bits >> 48));
  writeByte(serializer, (uint8_t)(bits >> 40));
  writeByte(serializer, (uint8_t)(bits >> 32));
  writeByte(serializer, (uint8_t)(bits >> 24));
  writeByte(serializer, (uint8_t)(bits >> 16));
  writeByte(serializer, (uint8_t)(bits >> 8));
  writeByte(serializer, (uint8_t)(bits));
}

static void writeString(Serializer* serializer, const char* bytes,
                        uint32_t length)
{
  writeUint32(serializer, length);
  for (uint32_t i = 0; i < length; i++)
  {
    writeByte(serializer, (uint8_t)bytes[i]);
  }
}

static void writeObjString(Serializer* serializer, ObjString* string)
{
  writeString(serializer, string->value, string->length);
}

static void serializeFunction(Serializer* serializer, ObjFn* fn);

static void serializeConstant(Serializer* serializer, Value constant)
{
  if (IS_NULL(constant))
  {
    writeByte(serializer, CONSTANT_NULL);
  }
  else if (IS_BOOL(constant))
  {
    writeByte(serializer, AS_BOOL(constant) ? CONSTANT_TRUE : CONSTANT_FALSE);
  }
  else if (IS_NUM(constant))
  {
    writeByte(serializer, CONSTANT_NUM);
    writeDouble(serializer, AS_NUM(constant));
  }
  else if (IS_STRING(constant))
  {
    writeByte(serializer, CONSTANT_STRING);
    writeObjString(serializer, AS_STRING(constant));
  }
  else if (IS_FN(constant))
  {
    writeByte(serializer, CONSTANT_FN);
    serializeFunction(serializer, AS_FN(constant));
  }
  else
  {
    // v1 only supports null, bools, numbers, strings, and nested functions.
    serializer->ok = false;
  }
}

static void serializeFunction(Serializer* serializer, ObjFn* fn)
{
  if (!serializer->ok) return;

  ASSERT(fn->code.count >= 0, "Code count must not be negative.");

  // Function metadata is written before code/constants so the loader can
  // validate it before allocating.
  writeByte(serializer, (uint8_t)fn->arity);
  writeUint16(serializer, (uint16_t)fn->numUpvalues);
  writeUint32(serializer, (uint32_t)fn->maxSlots);

  // Raw bytecode bytes, including any inline CODE_CLOSURE upvalue metadata.
  writeUint32(serializer, (uint32_t)fn->code.count);
  for (int i = 0; i < fn->code.count; i++)
  {
    writeByte(serializer, fn->code.data[i]);
  }

  // Constant table.
  writeUint32(serializer, (uint32_t)fn->constants.count);
  for (int i = 0; i < fn->constants.count; i++)
  {
    serializeConstant(serializer, fn->constants.data[i]);
    if (!serializer->ok) return;
  }

  // Debug information.
  if (serializer->debugInfo)
  {
    const char* name = fn->debug != NULL && fn->debug->name != NULL
        ? fn->debug->name
        : "";
    writeString(serializer, name, (uint32_t)strlen(name));

    int lineCount = fn->debug != NULL ? fn->debug->sourceLines.count : 0;
    writeUint32(serializer, (uint32_t)lineCount);
    for (int i = 0; i < lineCount; i++)
    {
      writeUint32(serializer, (uint32_t)fn->debug->sourceLines.data[i]);
    }
  }
}

WrenSerializeResult wrenSerializeModule(WrenConfiguration* configuration,
                                        const char* module,
                                        const char* source,
                                        bool debugInfo)
{
  (void)module;

  WrenSerializeResult result;
  result.bytes = NULL;
  result.length = 0;

  if (source == NULL) return result;

  WrenVM* vm = wrenNewVM(configuration);
  if (vm == NULL) return result;

  Serializer serializer;
  serializer.vm = vm;
  serializer.debugInfo = debugInfo;
  serializer.ok = true;
  wrenByteBufferInit(&serializer.buffer);

  // Find the core module (keyed by null in the VM's module map).
  ObjModule* coreModule = NULL;
  Value coreModuleValue = wrenMapGet(vm->modules, NULL_VAL);
  if (IS_OBJ(coreModuleValue) && AS_OBJ(coreModuleValue)->type == OBJ_MODULE)
  {
    coreModule = AS_MODULE(coreModuleValue);
  }

  if (coreModule == NULL)
  {
    wrenFreeVM(vm);
    return result;
  }

  // Create a fresh module and copy the core variables into it, mirroring
  // compileInModule. We deliberately do not register this module in the VM's
  // module map: the artifact carries no module name and v1 does not support
  // imports across serialized modules.
  ObjModule* moduleObj = wrenNewModule(vm, NULL);
  wrenPushRoot(vm, (Obj*)moduleObj);

  for (int i = 0; i < coreModule->variables.count; i++)
  {
    wrenDefineVariable(vm, moduleObj,
                       coreModule->variableNames.data[i]->value,
                       coreModule->variableNames.data[i]->length,
                       coreModule->variables.data[i], NULL);
  }

  // This is the only correct point to record the boundary between inherited
  // core variables and this module's own user-declared variables. After
  // compilation completes new variable names will be mixed in and there is no
  // way to tell them apart.
  int variableNameBoundary = moduleObj->variableNames.count;

  // Compile the source into the module using the normal compiler path.
  ObjFn* fn = wrenCompile(vm, moduleObj, source, false, true);
  if (fn == NULL)
  {
    wrenPopRoot(vm);
    wrenByteBufferClear(vm, &serializer.buffer);
    wrenFreeVM(vm);
    return result;
  }
  wrenPushRoot(vm, (Obj*)fn);

  // Artifact header.
  writeByte(&serializer, 'W');
  writeByte(&serializer, 'R');
  writeByte(&serializer, 'E');
  writeByte(&serializer, 'N');
  writeByte(&serializer, WREN_VERSION_MAJOR);
  writeByte(&serializer, WREN_VERSION_MINOR);
  writeByte(&serializer, WREN_VERSION_PATCH);
  writeByte(&serializer, debugInfo ? HEADER_FLAG_DEBUG_INFO : 0);

  // Module metadata: only the module's own user-declared top-level variable
  // names (and their count). Core-module names and values are intentionally
  // excluded.
  int ownVariableCount = moduleObj->variableNames.count - variableNameBoundary;
  writeUint32(&serializer, (uint32_t)ownVariableCount);
  for (int i = variableNameBoundary; i < moduleObj->variableNames.count; i++)
  {
    writeObjString(&serializer, moduleObj->variableNames.data[i]);
  }

  // Compiled function tree.
  serializeFunction(&serializer, fn);

  wrenPopRoot(vm); // fn.
  wrenPopRoot(vm); // moduleObj.

  if (serializer.ok && serializer.buffer.count > 0)
  {
    size_t length = (size_t)serializer.buffer.count;
    uint8_t* bytes = (uint8_t*)vm->config.reallocateFn(NULL, length,
                                                      vm->config.userData);
    if (bytes != NULL)
    {
      memcpy(bytes, serializer.buffer.data, length);
      result.bytes = bytes;
      result.length = length;
    }
  }

  wrenByteBufferClear(vm, &serializer.buffer);
  wrenFreeVM(vm);
  return result;
}

void wrenFreeSerializeResult(WrenConfiguration* configuration,
                             WrenSerializeResult result)
{
  if (result.bytes == NULL) return;

  if (configuration != NULL && configuration->reallocateFn != NULL)
  {
    configuration->reallocateFn(result.bytes, 0, configuration->userData);
  }
  else
  {
    free(result.bytes);
  }
}

// -----------------------------------------------------------------------------
// Bytecode loader
// -----------------------------------------------------------------------------

typedef struct
{
  const uint8_t* bytes;
  size_t length;
  size_t offset;
} ByteReader;

static void loadError(WrenVM* vm, const char* module, const char* message)
{
  if (vm->config.errorFn == NULL) return;

  vm->config.errorFn(vm, WREN_ERROR_LOAD, module, -1, message);
}

static bool hasBytes(ByteReader* reader, size_t count)
{
  return reader->offset + count <= reader->length;
}

static bool readByte(ByteReader* reader, uint8_t* byte)
{
  if (!hasBytes(reader, 1)) return false;

  *byte = reader->bytes[reader->offset++];
  return true;
}

static bool readUint16(ByteReader* reader, uint16_t* value)
{
  if (!hasBytes(reader, 2)) return false;

  *value = (uint16_t)((reader->bytes[reader->offset] << 8)
      | reader->bytes[reader->offset + 1]);
  reader->offset += 2;
  return true;
}

static bool readUint32(ByteReader* reader, uint32_t* value)
{
  if (!hasBytes(reader, 4)) return false;

  *value = ((uint32_t)reader->bytes[reader->offset] << 24)
         | ((uint32_t)reader->bytes[reader->offset + 1] << 16)
         | ((uint32_t)reader->bytes[reader->offset + 2] << 8)
         | ((uint32_t)reader->bytes[reader->offset + 3]);
  reader->offset += 4;
  return true;
}

static bool readDouble(ByteReader* reader, double* value)
{
  if (!hasBytes(reader, 8)) return false;

  uint64_t bits = ((uint64_t)reader->bytes[reader->offset] << 56)
                | ((uint64_t)reader->bytes[reader->offset + 1] << 48)
                | ((uint64_t)reader->bytes[reader->offset + 2] << 40)
                | ((uint64_t)reader->bytes[reader->offset + 3] << 32)
                | ((uint64_t)reader->bytes[reader->offset + 4] << 24)
                | ((uint64_t)reader->bytes[reader->offset + 5] << 16)
                | ((uint64_t)reader->bytes[reader->offset + 6] << 8)
                | ((uint64_t)reader->bytes[reader->offset + 7]);
  reader->offset += 8;
  *value = wrenDoubleFromBits(bits);
  return true;
}

static bool readObjString(ByteReader* reader, WrenVM* vm, ObjString** outString)
{
  uint32_t length;
  if (!readUint32(reader, &length)) return false;
  if (length > INT_MAX) return false;
  if (!hasBytes(reader, length)) return false;

  ObjString* string = AS_STRING(wrenNewStringLength(vm,
      (const char*)reader->bytes + reader->offset, length));
  reader->offset += length;

  *outString = string;
  return true;
}

// Reads a function's leading metadata and allocates an empty ObjFn. Validation
// happens before the allocation so a malformed artifact cannot force an
// invalid function shape.
static bool allocateFunction(ByteReader* reader, WrenVM* vm, ObjModule* module,
                             ObjFn** outFn)
{
  uint8_t arity;
  uint16_t numUpvalues;
  uint32_t maxSlots;

  if (!readByte(reader, &arity)) return false;
  if (!readUint16(reader, &numUpvalues)) return false;
  if (!readUint32(reader, &maxSlots)) return false;

  if (arity > MAX_PARAMETERS) return false;
  if (numUpvalues > MAX_UPVALUES) return false;
  if (maxSlots == 0 || maxSlots < (uint32_t)arity + 1) return false;

  ObjFn* fn = wrenNewFunction(vm, module, (int)maxSlots);
  fn->arity = (int)arity;
  fn->numUpvalues = (int)numUpvalues;

  *outFn = fn;
  return true;
}

static bool loadFunctionBody(ByteReader* reader, WrenVM* vm, ObjModule* module,
                             bool debugInfo, ObjFn* fn);

// Loads a function body. [fn] must be reachable through a temp root or an
// already-rooted parent function's constants table so nested allocations do not
// collect it.
static bool readConstant(ByteReader* reader, WrenVM* vm, ObjModule* module,
                         bool debugInfo, ObjFn* fn)
{
  uint8_t tag;
  if (!readByte(reader, &tag)) return false;

  switch (tag)
  {
    case CONSTANT_NULL:
      wrenValueBufferWrite(vm, &fn->constants, NULL_VAL);
      break;

    case CONSTANT_FALSE:
      wrenValueBufferWrite(vm, &fn->constants, FALSE_VAL);
      break;

    case CONSTANT_TRUE:
      wrenValueBufferWrite(vm, &fn->constants, TRUE_VAL);
      break;

    case CONSTANT_NUM:
    {
      double value;
      if (!readDouble(reader, &value)) return false;
      wrenValueBufferWrite(vm, &fn->constants, NUM_VAL(value));
      break;
    }

    case CONSTANT_STRING:
    {
      ObjString* string;
      if (!readObjString(reader, vm, &string)) return false;

      wrenPushRoot(vm, (Obj*)string);
      wrenValueBufferWrite(vm, &fn->constants, OBJ_VAL(string));
      wrenPopRoot(vm);
      break;
    }

    case CONSTANT_FN:
    {
      ObjFn* child;
      if (!allocateFunction(reader, vm, module, &child)) return false;

      // Attach-before-fill: the child is reachable through [fn]'s constants
      // table before we recurse to fill its body.
      wrenPushRoot(vm, (Obj*)child);
      wrenValueBufferWrite(vm, &fn->constants, OBJ_VAL(child));
      wrenPopRoot(vm);

      if (!loadFunctionBody(reader, vm, module, debugInfo, child)) return false;
      break;
    }

    default:
      return false;
  }

  return true;
}

static bool loadFunctionBody(ByteReader* reader, WrenVM* vm, ObjModule* module,
                             bool debugInfo, ObjFn* fn)
{
  uint32_t codeLength;
  if (!readUint32(reader, &codeLength)) return false;
  if (codeLength > INT_MAX) return false;
  if (!hasBytes(reader, codeLength)) return false;

  for (uint32_t i = 0; i < codeLength; i++)
  {
    wrenByteBufferWrite(vm, &fn->code, reader->bytes[reader->offset++]);
  }

  uint32_t constantCount;
  if (!readUint32(reader, &constantCount)) return false;
  if (constantCount > MAX_CONSTANTS) return false;

  for (uint32_t i = 0; i < constantCount; i++)
  {
    if (!readConstant(reader, vm, module, debugInfo, fn)) return false;
  }

  // Debug information (or a safe fallback).
  if (debugInfo)
  {
    ObjString* name;
    if (!readObjString(reader, vm, &name)) return false;

    wrenPushRoot(vm, (Obj*)name);
    wrenFunctionBindName(vm, fn, name->value, (int)name->length);
    wrenPopRoot(vm);

    uint32_t lineCount;
    if (!readUint32(reader, &lineCount)) return false;
    if (lineCount != codeLength) return false;
    if (!hasBytes(reader, lineCount * 4)) return false;

    for (uint32_t i = 0; i < lineCount; i++)
    {
      uint32_t line;
      if (!readUint32(reader, &line)) return false;
      if (line > INT_MAX) return false;

      wrenIntBufferWrite(vm, &fn->debug->sourceLines, (int)line);
    }
  }
  else
  {
    wrenFunctionBindName(vm, fn, "(bytecode)", 10);
    wrenIntBufferFill(vm, &fn->debug->sourceLines, 0, (int)codeLength);
  }

  return true;
}

WrenInterpretResult wrenInterpretBytecode(WrenVM* vm, const char* module,
                                          const uint8_t* bytes, size_t length)
{
  if (module == NULL)
  {
    loadError(vm, NULL, "Module name cannot be null.");
    return WREN_RESULT_LOAD_ERROR;
  }

  if (bytes == NULL)
  {
    loadError(vm, module, "Bytecode buffer cannot be null.");
    return WREN_RESULT_LOAD_ERROR;
  }

  ByteReader reader;
  reader.bytes = bytes;
  reader.length = length;
  reader.offset = 0;

  // Header: magic (4) + version (3) + flags (1) = 8 bytes.
  if (!hasBytes(&reader, 8))
  {
    loadError(vm, module, "Bytecode artifact is truncated.");
    return WREN_RESULT_LOAD_ERROR;
  }

  if (memcmp(bytes, "WREN", 4) != 0)
  {
    loadError(vm, module, "Bytecode artifact has invalid magic.");
    return WREN_RESULT_LOAD_ERROR;
  }

  if (bytes[4] != WREN_VERSION_MAJOR ||
      bytes[5] != WREN_VERSION_MINOR ||
      bytes[6] != WREN_VERSION_PATCH)
  {
    loadError(vm, module, "Bytecode artifact version does not match VM.");
    return WREN_RESULT_LOAD_ERROR;
  }

  uint8_t flags = bytes[7];
  if ((flags & ~HEADER_FLAG_DEBUG_INFO) != 0)
  {
    loadError(vm, module, "Bytecode artifact has unknown header flags.");
    return WREN_RESULT_LOAD_ERROR;
  }

  bool debugInfo = (flags & HEADER_FLAG_DEBUG_INFO) != 0;
  reader.offset = 8;

  // Reject an already-loaded module name before allocating anything.
  if (wrenHasModule(vm, module))
  {
    loadError(vm, module, "Module is already loaded.");
    return WREN_RESULT_LOAD_ERROR;
  }

  // Create and root the module name before creating the module, so the name is
  // reachable if module creation triggers a GC.
  Value nameValue = wrenNewString(vm, module);
  wrenPushRoot(vm, AS_OBJ(nameValue));

  ObjModule* moduleObj = wrenNewModule(vm, AS_STRING(nameValue));
  wrenPushRoot(vm, (Obj*)moduleObj);

  // Copy core-module variables into the new module, matching compileInModule.
  ObjModule* coreModule = NULL;
  Value coreModuleValue = wrenMapGet(vm->modules, NULL_VAL);
  if (IS_OBJ(coreModuleValue) && AS_OBJ(coreModuleValue)->type == OBJ_MODULE)
  {
    coreModule = AS_MODULE(coreModuleValue);
  }

  // Without a core module the VM is in an invalid state, so treat it as a load
  // error instead of crashing.
  if (coreModule == NULL)
  {
    loadError(vm, module, "Could not find core module.");
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  for (int i = 0; i < coreModule->variables.count; i++)
  {
    int result = wrenDefineVariable(vm, moduleObj,
                                    coreModule->variableNames.data[i]->value,
                                    coreModule->variableNames.data[i]->length,
                                    coreModule->variables.data[i], NULL);
    // Defining core variables should never collide because the module is fresh,
    // but treat any failure defensively.
    if (result < 0)
    {
      loadError(vm, module, "Could not copy core module variables.");
      wrenPopRoot(vm); // module.
      wrenPopRoot(vm); // name.
      return WREN_RESULT_LOAD_ERROR;
    }
  }

  // Read the module's own variable names and reserve slots for them.
  uint32_t ownVariableCount;
  if (!readUint32(&reader, &ownVariableCount))
  {
    loadError(vm, module, "Bytecode artifact is truncated.");
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  size_t maxOwnVariables = (size_t)INT_MAX;
  if (ownVariableCount > maxOwnVariables)
  {
    loadError(vm, module, "Too many module variables.");
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  for (uint32_t i = 0; i < ownVariableCount; i++)
  {
    ObjString* name;
    if (!readObjString(&reader, vm, &name))
    {
      loadError(vm, module, "Bytecode artifact is truncated.");
      wrenPopRoot(vm); // module.
      wrenPopRoot(vm); // name.
      return WREN_RESULT_LOAD_ERROR;
    }

    if (name->length == 0 || name->length > MAX_VARIABLE_NAME)
    {
      loadError(vm, module, "Invalid module variable name length.");
      wrenPopRoot(vm); // module.
      wrenPopRoot(vm); // name.
      return WREN_RESULT_LOAD_ERROR;
    }

    int result = wrenDefineVariable(vm, moduleObj, name->value, name->length,
                                    NULL_VAL, NULL);
    if (result < 0)
    {
      loadError(vm, module, "Duplicate or invalid module variable name.");
      wrenPopRoot(vm); // module.
      wrenPopRoot(vm); // name.
      return WREN_RESULT_LOAD_ERROR;
    }
  }

  // Rebuild the function tree. The root function remains rooted until closure
  // creation is complete.
  ObjFn* rootFn;
  if (!allocateFunction(&reader, vm, moduleObj, &rootFn))
  {
    loadError(vm, module, "Invalid root function metadata.");
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  wrenPushRoot(vm, (Obj*)rootFn);

  if (!loadFunctionBody(&reader, vm, moduleObj, debugInfo, rootFn))
  {
    loadError(vm, module, "Invalid function bytecode or constants.");
    wrenPopRoot(vm); // rootFn.
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  // The root function is created directly as a closure by the loader, so it
  // cannot close over any enclosing scope's upvalues.
  if (rootFn->numUpvalues != 0)
  {
    loadError(vm, module, "Root function cannot have upvalues.");
    wrenPopRoot(vm); // rootFn.
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  // The artifact should have no trailing bytes after the root function tree.
  if (reader.offset != reader.length)
  {
    loadError(vm, module, "Bytecode artifact has trailing bytes.");
    wrenPopRoot(vm); // rootFn.
    wrenPopRoot(vm); // module.
    wrenPopRoot(vm); // name.
    return WREN_RESULT_LOAD_ERROR;
  }

  // All structural validation succeeded. Register the module before execution.
  wrenMapSet(vm, vm->modules, nameValue, OBJ_VAL(moduleObj));

  // The name and module roots are now below the root function root on the
  // LIFO temp-root stack. Reorder without allocating: release the module/name
  // roots while keeping the root function rooted for closure creation.
  wrenPopRoot(vm); // rootFn.
  wrenPopRoot(vm); // module.
  wrenPopRoot(vm); // name.
  wrenPushRoot(vm, (Obj*)rootFn);

  ObjClosure* closure = wrenNewClosure(vm, rootFn);
  wrenPopRoot(vm); // rootFn, now held by the closure.

  return wrenRunClosure(vm, closure);
}
