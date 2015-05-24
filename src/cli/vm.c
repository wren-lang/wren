#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"

WrenVM* createVM(WrenBindForeignMethodFn bindForeign)
{
  WrenConfiguration config;

  config.bindForeignMethodFn = bindForeign;
  config.loadModuleFn = readModule;

  // Since we're running in a standalone process, be generous with memory.
  config.initialHeapSize = 1024 * 1024 * 100;

  // Use defaults for these.
  config.reallocateFn = NULL;
  config.minHeapSize = 0;
  config.heapGrowthPercent = 0;

  return wrenNewVM(&config);
}

void runFile(WrenBindForeignMethodFn bindForeign, const char* path)
{
  // Use the directory where the file is as the root to resolve imports
  // relative to.
  const char* lastSlash = strrchr(path, '/');
  if (lastSlash != NULL)
  {
    char* root = (char*)malloc(lastSlash - path + 2);
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

  WrenVM* vm = createVM(bindForeign);

  WrenInterpretResult result = wrenInterpret(vm, path, source);

  wrenFreeVM(vm);
  free(source);

  // Exit with an error code if the script failed.
  if (result == WREN_RESULT_COMPILE_ERROR) exit(65); // EX_DATAERR.
  if (result == WREN_RESULT_RUNTIME_ERROR) exit(70); // EX_SOFTWARE.
}
