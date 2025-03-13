#include <stdio.h>
#include <string.h>

#include "wren.h"

static void myVMWrite(WrenVM* vm, const char* text)
{
  printf("%s", text);
}

static void myReportError(WrenVM* vm, WrenErrorType type,
  const char* module, int line, const char* message)
{
  switch (type)
  {
    case WREN_ERROR_COMPILE:
      fprintf(stderr, "[%s line %d] %s\n", module, line, message);
      break;

    case WREN_ERROR_RUNTIME:
      fprintf(stderr, "%s\n", message);
      break;

    case WREN_ERROR_STACK_TRACE:
      fprintf(stderr, "[%s line %d] in %s\n", module, line, message);
      break;
  }
}

static void setled(const unsigned int v)
{
  printf("\tIn %s(%u)\n", __func__, v);
}

static void led(WrenVM* vm)
{
  printf("\tIn %s()\n", __func__);

  const double color = wrenGetSlotDouble(vm, 1);

  setled((int)color);
}

static WrenLoadModuleResult imports =
{
  .onComplete = NULL,
  .userData = NULL,
  .source = "\
var Toto = 4242\n\
"
};

static WrenLoadModuleResult loadModule(WrenVM* vm, const char* name)
{
  printf("\tIn %s()\n", __func__);

  WrenLoadModuleResult wlmr = {0};
  if (strcmp(name, "ZZZ") == 0)
  {
    wlmr = imports;
  }
  return wlmr;
}

static WrenForeignMethodFn bindForeignMethod(
  WrenVM* vm,
  const char* module,
  const char* className,
  bool isStatic,
  const char* signature
)
{
  printf(
    "\tIn %s(module => %s, class => %s, static => %s, signature => %s)\n",
    __func__, module, className, isStatic ? "y" : "", signature
  );

  if (strcmp(module, "./button-led") == 0)
  {
      if (isStatic)
      {
          if (strcmp(className, "HW") == 0)
          {
              if (strcmp(signature, "led(_)") == 0)
              {
                  return led;
              }
          }
      }
  }
  return NULL;
}

static const char* WrenResultToString[] = {
  [WREN_RESULT_SUCCESS]       = "WREN_RESULT_SUCCESS",
  [WREN_RESULT_COMPILE_ERROR] = "WREN_RESULT_COMPILE_ERROR",
  [WREN_RESULT_RUNTIME_ERROR] = "WREN_RESULT_RUNTIME_ERROR",
};

WrenHandle* wClass;
WrenHandle* wFuncB;
WrenHandle* wFuncI;
WrenVM* newVM;

static void demoForeign(WrenVM* vm)
{
  static const char resolvedModule[] = "./button-led";
  static const char className[] = "Main";
  static const char funcSignatureB[] = "BTN()";
  static const char funcSignatureI[] = "Init()";

  printf("\tIn %s()\n", __func__);

  // Get a handle on the class.
  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, resolvedModule, className, 0);
  wClass = wrenGetSlotHandle(vm, 0);

  // Get a handle on the func.
  wFuncI = wrenMakeCallHandle(vm, funcSignatureI);

  // Get a handle on the func.
  wFuncB = wrenMakeCallHandle(vm, funcSignatureB);
}

static void performRestore(const char* fileName)
{
  FILE* file = fopen(fileName, "rb");
  if (file == NULL) return;

  printf("=== wrenNewEmptyVM\n");

  WrenConfiguration newConfig;
  wrenInitConfiguration(&newConfig);
  newConfig.userData = "VM restored from a snapshot";
  newConfig.initialHeapSize = 128 * 1024;
  newConfig.minHeapSize     = 128 * 1024;
  newConfig.writeFn = myVMWrite;
  newConfig.errorFn = myReportError;
  newConfig.loadModuleFn = loadModule;
  newConfig.bindForeignMethodFn = bindForeignMethod;

  newVM = wrenNewEmptyVM(&newConfig);

  printf("=== performRestore\n");
  ObjClosure* entrypoint = wrenSnapshotRestore(file, newVM);

  fclose(file);

  if (entrypoint == NULL) return;

  printf("=== start new VM\n");
  WrenInterpretResult result = wrenInterpretClosure(newVM, entrypoint);
  printf("=== result %s\n", WrenResultToString[result]);

  printf("=== use new VM\n");

  demoForeign(newVM);

  // Call the method on the class.
  wrenEnsureSlots(newVM, 1);
  wrenSetSlotHandle(newVM, 0, wClass);
  result = wrenCall(newVM, wFuncI);
  printf("\twrenCall() => %s\n", WrenResultToString[result]);

  // Call the method on the class.
  wrenEnsureSlots(newVM, 1);
  wrenSetSlotHandle(newVM, 0, wClass);
  result = wrenCall(newVM, wFuncB);
  printf("\twrenCall() => %s\n", WrenResultToString[result]);

  printf("=== cleanup\n");

  wrenReleaseHandle(newVM, wClass);
  wrenReleaseHandle(newVM, wFuncI);
  wrenReleaseHandle(newVM, wFuncB);

  wrenFreeVM(newVM);
}

int main(int argc, const char* argv[]) {
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s WREN-BYTECODE-BIN-FILE\n", argv[0]);
    return 1;
  }

  performRestore(argv[1]);

  return 0;
}
