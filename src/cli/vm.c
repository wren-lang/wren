#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"
#include "timer.h"

#define MAX_LINE_LENGTH 1024 // TODO: Something less arbitrary.

// The single VM instance that the CLI uses.
WrenVM* vm;

WrenBindForeignMethodFn externalBindForeign;

uv_loop_t* loop;

static WrenForeignMethodFn bindForeignMethod(WrenVM* vm,
                                             const char* module,
                                             const char* className,
                                             bool isStatic,
                                             const char* signature)
{
  if (strcmp(module, "timer") == 0)
  {
    return timerBindForeign(vm, className, isStatic, signature);
  }
  
  if (externalBindForeign != NULL)
  {
    return externalBindForeign(vm, module, className, isStatic, signature);
  }
  
  return NULL;
}

static void initVM()
{
  WrenConfiguration config;

  config.bindForeignMethodFn = bindForeignMethod;
  config.loadModuleFn = readModule;

  // Since we're running in a standalone process, be generous with memory.
  config.initialHeapSize = 1024 * 1024 * 100;

  // Use defaults for these.
  config.reallocateFn = NULL;
  config.minHeapSize = 0;
  config.heapGrowthPercent = 0;
  
  vm = wrenNewVM(&config);
  
  // Initialize the event loop.
  loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);
}

static void freeVM()
{
  timerReleaseMethods();
  
  uv_loop_close(loop);
  free(loop);

  wrenFreeVM(vm);
}

void runFile(const char* path, WrenBindForeignMethodFn bindForeign)
{
  externalBindForeign = bindForeign;
  
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
