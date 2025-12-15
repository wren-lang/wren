#include "wren_opt_mirror.h"

#if WREN_OPT_MIRROR

#include <string.h>

#include "wren_vm.h"
#include "wren_opt_mirror.wren.inc"

static ObjClass* mirrorGetSlotClass(WrenVM* vm, int slot)
{
  Value classVal = *wrenSlotAtUnsafe(vm, slot);
  if (!IS_CLASS(classVal)) return NULL;

  return AS_CLASS(classVal);
}

static ObjClosure* mirrorGetSlotClosure(WrenVM* vm, int slot)
{
  Value closureVal = *wrenSlotAtUnsafe(vm, slot);
  if (!IS_CLOSURE(closureVal)) return NULL;

  return AS_CLOSURE(closureVal);
}

static ObjFiber* mirrorGetSlotFiber(WrenVM* vm, int slot)
{
  Value fiberVal = *wrenSlotAtUnsafe(vm, slot);
  if (!IS_FIBER(fiberVal)) return NULL;

  return AS_FIBER(fiberVal);
}

static ObjModule* mirrorGetSlotModule(WrenVM* vm, int slot)
{
  Value moduleVal = *wrenSlotAtUnsafe(vm, slot);
  if (!IS_MODULE(moduleVal)) return NULL;

  return AS_MODULE(moduleVal);
}

static void mirrorClassMirrorHasMethod(WrenVM* vm)
{
  ObjClass* classObj = mirrorGetSlotClass(vm, 1);
  const char* signature = wrenGetSlotString(vm, 2);

  bool hasMethod = false;
  if (classObj != NULL &&
      signature != NULL)
  {
    int symbol = wrenSymbolTableFind(&vm->methodNames,
                                     signature, strlen(signature));
    hasMethod = wrenClassGetMethod(vm, classObj, symbol) != NULL;
  }
  wrenSetSlotBool(vm, 0, hasMethod);
}

static void mirrorClassMirrorMethodNames(WrenVM* vm)
{
  ObjClass* classObj = mirrorGetSlotClass(vm, 1);

  if (!classObj)
  {
    wrenSetSlotNull(vm, 0);
    return;
  }

  wrenSetSlotNewList(vm, 0);
  for (size_t symbol = 0; symbol < classObj->methods.count; symbol++)
  {
    Method* method = wrenClassGetMethod(vm, classObj, symbol);
    if (method == NULL) continue;

    *wrenSlotAtUnsafe(vm, 1) = OBJ_VAL(vm->methodNames.data[symbol]);
    wrenInsertInList(vm, 0, -1, 1);
  }
}

static void mirrorClassMirrorModule_(WrenVM* vm)
{
  ObjClass* classObj = mirrorGetSlotClass(vm, 1);

  ASSERT(classObj, "Slot must hold a class");

  // [classObj->module] can be NULL with a core class
  *wrenSlotAtUnsafe(vm, 0) = classObj->module ? OBJ_VAL(classObj->module)
                                              : NULL_VAL;
}

static void mirrorClosureMirrorBoundToClass_(WrenVM* vm)
{
  ObjClosure* closureObj = mirrorGetSlotClosure(vm, 1);

  ASSERT(closureObj, "Slot must hold a closure");
  ASSERT(closureObj->fn, "Closure must hold a function");

  if (!closureObj->fn->debug->boundToClass)
  {
    *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(vm->fnClass);
    return;
  }

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(closureObj->fn->debug->boundToClass);
}

static void mirrorClosureMirrorModule_(WrenVM* vm)
{
  ObjClosure* closureObj = mirrorGetSlotClosure(vm, 1);

  ASSERT(closureObj, "Slot must hold a closure");
  ASSERT(closureObj->fn, "Closure must hold a function");
  ASSERT(closureObj->fn->module, "Function must hold a module");

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(closureObj->fn->module);
}

static void mirrorClosureMirrorSignature_(WrenVM* vm)
{
  ObjClosure* closureObj = mirrorGetSlotClosure(vm, 1);

  ASSERT(closureObj, "Slot must hold a closure");

  wrenSetSlotString(vm, 0, closureObj->fn->debug->name);
}

static CallFrame* mirrorFiberMirrorFrameAt_(WrenVM* vm)
{
  ObjFiber* fiber = mirrorGetSlotFiber(vm, 1);
  size_t frameIndex = wrenGetSlotDouble(vm, 2);

  ASSERT(fiber, "Slot must hold a fiber");
  ASSERT(fiber->numFrames > 0, "Fiber must have frames");
  ASSERT(frameIndex < fiber->numFrames, "Out of bound frame index");

  CallFrame* frame = &fiber->frames[frameIndex];

  ASSERT(frame->closure, "Frame must hold a closure");
  ASSERT(frame->closure->fn, "Closure must hold a function");

  return frame;
}

static void mirrorFiberMirrorClosureAt_(WrenVM* vm)
{
  CallFrame* frame = mirrorFiberMirrorFrameAt_(vm);

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(frame->closure);
}

static void mirrorFiberMirrorLineAt_(WrenVM* vm)
{
  CallFrame* frame = mirrorFiberMirrorFrameAt_(vm);

  ObjFn* fn = frame->closure->fn;
  int* sourceLines = fn->debug->sourceLines.data;
  if (sourceLines == NULL)
  {
    wrenSetSlotNull(vm, 0);
    return;
  }

  wrenSetSlotDouble(vm, 0, sourceLines[frame->ip - fn->code.data]);
}

static void mirrorFiberMirrorStackFramesCount_(WrenVM* vm)
{
  ObjFiber* fiber = mirrorGetSlotFiber(vm, 1);

  ASSERT(fiber, "Slot must hold a fiber");

  wrenSetSlotDouble(vm, 0, fiber->numFrames);
}

static void mirrorMethodMirrorSignature_(WrenVM* vm)
{
  size_t signatureIndex = wrenGetSlotDouble(vm, 1);

  ASSERT(signatureIndex < vm->methodNames.count, "Out of bounds symbol index.");

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(vm->methodNames.data[signatureIndex]);
}

static void mirrorModuleMirrorCurrent_(WrenVM* vm)
{
  ObjFiber* fiberObj = vm->fiber;
  CallFrame* frame = &fiberObj->frames[fiberObj->numFrames - 1];
  ObjModule* moduleObj = frame->closure->fn->module;

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(moduleObj);
}

static void mirrorModuleMirrorFromName_(WrenVM* vm)
{
  const char* moduleName = wrenGetSlotString(vm, 1);

  if (!moduleName)
  {
    wrenSetSlotNull(vm, 0);
    return;
  }

  // Special case for "core"
  if (strcmp(moduleName, "core") == 0)
  {
    wrenSetSlotNull(vm, 1);
  }

  ObjModule* module = wrenGetModule(vm, *wrenSlotAtUnsafe(vm, 1));
  if (module != NULL)
  {
    *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(module);
  }
  else
  {
    wrenSetSlotNull(vm, 0);
  }
}

static void mirrorModuleMirrorName_(WrenVM* vm)
{
  ObjModule* moduleObj = mirrorGetSlotModule(vm, 1);

  ASSERT(moduleObj, "Slot must hold a module");

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(moduleObj->name);
}

static void mirrorObjectMirrorCanInvoke(WrenVM* vm)
{
  ObjClass* classObj = wrenGetClassInline(vm, *wrenSlotAtUnsafe(vm, 1));

  *wrenSlotAtUnsafe(vm, 1) = OBJ_VAL(classObj);
  mirrorClassMirrorHasMethod(vm);
}

static void mirrorObjectMirrorTypeOf(WrenVM* vm)
{
  ObjClass* classObj = wrenGetClassInline(vm, *wrenSlotAtUnsafe(vm, 1));

  *wrenSlotAtUnsafe(vm, 0) = OBJ_VAL(classObj);
}

const char* wrenMirrorSource()
{
  return mirrorModuleSource;
}

WrenForeignMethodFn wrenMirrorBindForeignMethod(WrenVM* vm,
                                                const char* className,
                                                bool isStatic,
                                                const char* signature)
{
  if (strcmp(className, "ClassMirror") == 0)
  {
    if (isStatic &&
        strcmp(signature, "hasMethod(_,_)") == 0)
    {
      return mirrorClassMirrorHasMethod;
    }
    if (isStatic &&
        strcmp(signature, "methodNames(_)") == 0)
    {
      return mirrorClassMirrorMethodNames;
    }
    if (isStatic &&
        strcmp(signature, "module_(_)") == 0)
    {
      return mirrorClassMirrorModule_;
    }
  }

  if (strcmp(className, "ClosureMirror") == 0)
  {
    if (isStatic &&
        strcmp(signature, "boundToClass_(_)") == 0)
    {
      return mirrorClosureMirrorBoundToClass_;
    }
    if (isStatic &&
        strcmp(signature, "module_(_)") == 0)
    {
      return mirrorClosureMirrorModule_;
    }
    if (isStatic &&
        strcmp(signature, "signature_(_)") == 0)
    {
      return mirrorClosureMirrorSignature_;
    }
  }

  if (strcmp(className, "FiberMirror") == 0)
  {
    if (isStatic &&
        strcmp(signature, "closureAt_(_,_)") == 0)
    {
      return mirrorFiberMirrorClosureAt_;
    }
    if (isStatic &&
        strcmp(signature, "lineAt_(_,_)") == 0)
    {
      return mirrorFiberMirrorLineAt_;
    }
    if (isStatic &&
        strcmp(signature, "stackFramesCount_(_)") == 0)
    {
      return mirrorFiberMirrorStackFramesCount_;
    }
  }

  if (strcmp(className, "MethodMirror") == 0)
  {
    if (isStatic &&
        strcmp(signature, "signature_(_)") == 0)
    {
      return mirrorMethodMirrorSignature_;
    }
  }

  if (strcmp(className, "ModuleMirror") == 0)
  {
    if (isStatic &&
        strcmp(signature, "current_") == 0)
    {
      return mirrorModuleMirrorCurrent_;
    }
    if (isStatic &&
        strcmp(signature, "fromName_(_)") == 0)
    {
      return mirrorModuleMirrorFromName_;
    }
    if (isStatic &&
        strcmp(signature, "name_(_)") == 0)
    {
      return mirrorModuleMirrorName_;
    }
  }

  if (strcmp(className, "ObjectMirror") == 0)
  {
    if (isStatic &&
        strcmp(signature, "canInvoke(_,_)") == 0)
    {
      return mirrorObjectMirrorCanInvoke;
    }
    if (isStatic &&
        strcmp(signature, "typeOf(_)") == 0)
    {
      return mirrorObjectMirrorTypeOf;
    }
  }

  ASSERT(false, "Unknown method.");
  return NULL;
}

#endif
