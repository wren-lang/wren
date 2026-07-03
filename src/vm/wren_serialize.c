#include <string.h>
#include <stdlib.h>

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

  // Function metadata. All three values are bounded well below 256 by the
  // compiler.
  writeByte(serializer, (uint8_t)fn->arity);
  writeByte(serializer, (uint8_t)fn->numUpvalues);
  writeByte(serializer, (uint8_t)fn->maxSlots);

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
