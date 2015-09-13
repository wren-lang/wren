#include <stdio.h>
#include <string.h>

#include "io.h"
#include "modules.h"
#include "vm.h"
#include "scheduler.h"

#define MAX_LINE_LENGTH 1024 // TODO: Something less arbitrary.

// The single VM instance that the CLI uses.
WrenVM* vm;

static WrenBindForeignMethodFn bindMethodFn = NULL;
static WrenBindForeignClassFn bindClassFn = NULL;

uv_loop_t* loop;

// Binds foreign methods declared in either built in modules, or the injected
// API test modules.
static WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module,
    const char* className, bool isStatic, const char* signature)
{
  WrenForeignMethodFn method = bindBuiltInForeignMethod(vm, module, className,
                                                        isStatic, signature);
  if (method != NULL) return method;
  
  if (bindMethodFn != NULL)
  {
    return bindMethodFn(vm, module, className, isStatic, signature);
  }

  return NULL;
}

// Binds foreign classes declared in either built in modules, or the injected
// API test modules.
static WrenForeignClassMethods bindForeignClass(
    WrenVM* vm, const char* module, const char* className)
{
  WrenForeignClassMethods methods = bindBuiltInForeignClass(vm, module,
                                                            className);
  if (methods.allocate != NULL) return methods;

  if (bindClassFn != NULL)
  {
    return bindClassFn(vm, module, className);
  }

  return methods;
}

static void initVM()
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);

  config.bindForeignMethodFn = bindForeignMethod;
  config.bindForeignClassFn = bindForeignClass;
  config.loadModuleFn = readModule;

  // Since we're running in a standalone process, be generous with memory.
  config.initialHeapSize = 1024 * 1024 * 100;
  vm = wrenNewVM(&config);

  // Initialize the event loop.
  loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);
}

static void freeVM()
{
  schedulerReleaseMethods();

  uv_loop_close(loop);
  free(loop);

  wrenFreeVM(vm);
}

void runFile(const char* path)
{
  // Use the directory where the file is as the root to resolve imports
  // relative to.
  char* root = NULL;
  const char* lastSlash = strrchr(path, '/');
  if (lastSlash != NULL)
  {
    root = (char*)malloc(lastSlash - path + 2);
    memcpy(root, path, lastSlash - path + 1);
    root[lastSlash - path + 1] = '\0';
    setRootDirectory(root);
  }

  char* source = readFile(path);
  if (source == NULL)
  {
    fprintf(stderr, "Could not find file \"%s\".\n", path);
    exit(66);
  }

  initVM();

  WrenInterpretResult result = wrenInterpret(vm, path, source);

  if (result == WREN_RESULT_SUCCESS)
  {
    uv_run(loop, UV_RUN_DEFAULT);
  }

  freeVM();

  free(source);
  free(root);

  // Exit with an error code if the script failed.
  if (result == WREN_RESULT_COMPILE_ERROR) exit(65); // EX_DATAERR.
  if (result == WREN_RESULT_RUNTIME_ERROR) exit(70); // EX_SOFTWARE.
}

int runRepl()
{
  initVM();

  printf("\\\\/\"-\n");
  printf(" \\_/   wren v0.0.0\n");

  char line[MAX_LINE_LENGTH];

  for (;;)
  {
    printf("> ");

    if (!fgets(line, MAX_LINE_LENGTH, stdin))
    {
      printf("\n");
      break;
    }

    // TODO: Handle failure.
    wrenInterpret(vm, "Prompt", line);

    // TODO: Automatically print the result of expressions.
  }

  freeVM();

  return 0;
}

WrenVM* getVM()
{
  return vm;
}

uv_loop_t* getLoop()
{
  return loop;
}

void setForeignCallbacks(
    WrenBindForeignMethodFn bindMethod, WrenBindForeignClassFn bindClass)
{
  bindMethodFn = bindMethod;
  bindClassFn = bindClass;
}
