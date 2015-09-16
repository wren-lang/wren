#include <stdlib.h>
#include <string.h>

#include "modules.h"

#include "io.wren.inc"
#include "scheduler.wren.inc"
#include "timer.wren.inc"

#include "io.h"
#include "scheduler.h"
#include "timer.h"

typedef struct
{
  // The name of the module.
  const char* name;
  
  // Pointer to the string containing the source code of the module. We use a
  // pointer here because the string variable itself is not a constant
  // expression so can't be used in the initializer below.
  const char **source;

  // The function that binds foreign methods in this module.
  WrenForeignMethodFn (*bindMethodFn)(WrenVM* vm, const char* className,
                                      bool isStatic, const char* signature);

  // The function that binds foreign classes in this module.
  WrenForeignClassMethods (*bindClassFn)(WrenVM* vm, const char* className);
} BuiltInModule;

// The array of built-in modules.
static BuiltInModule modules[] =
{
  {"io",        &ioModuleSource,        ioBindForeign,        NULL},
  {"scheduler", &schedulerModuleSource, schedulerBindForeign, NULL},
  {"timer",     &timerModuleSource,     timerBindForeign,     NULL},
  
  // Sentinel marking the end of the list.
  {NULL, NULL, NULL, NULL}
};

// Looks for a built-in module with [name].
//
// Returns the BuildInModule for it or NULL if not found.
static BuiltInModule* findModule(const char* name)
{
  for (int i = 0; modules[i].name != NULL; i++)
  {
    if (strcmp(name, modules[i].name) == 0) return &modules[i];
  }
  
  return NULL;
}

char* readBuiltInModule(const char* name)
{
  BuiltInModule* module = findModule(name);
  if (module == NULL) return NULL;
  
  size_t length = strlen(*module->source);
  char* copy = (char*)malloc(length + 1);
  strncpy(copy, *module->source, length + 1);
  return copy;
}

WrenForeignMethodFn bindBuiltInForeignMethod(
    WrenVM* vm, const char* moduleName, const char* className, bool isStatic,
    const char* signature)
{
  BuiltInModule* module = findModule(moduleName);
  if (module == NULL) return NULL;
  
  return module->bindMethodFn(vm, className, isStatic, signature);
}

WrenForeignClassMethods bindBuiltInForeignClass(
    WrenVM* vm, const char* moduleName, const char* className)
{
  WrenForeignClassMethods methods = { NULL, NULL };
  
  BuiltInModule* module = findModule(moduleName);
  if (module == NULL) return methods;

  return module->bindClassFn(vm, className);
}