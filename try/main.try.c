#include "../test/test.h"

#include <stdio.h>
#include <string.h>

static WrenVM* vm = NULL;

//This is a simple program that exposes wren to the browser
//for https://wren.io/try and runs scripts.

static WrenVM* initVM()
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);

  config.resolveModuleFn = resolveModule;
  config.loadModuleFn = readModule;
  config.writeFn = vm_write;
  config.errorFn = reportError;

  // Might be a more reasonable value, 
  // but since this is simple, keep it simple.
  config.initialHeapSize = 1024 * 1024 * 100;
  return wrenNewVM(&config);
}

//The endpoint we call from the browser
int wren_compile(const char* input) {
  WrenVM* vm = initVM();
  WrenInterpretResult result = wrenInterpret(vm, "compile", input);
  wrenFreeVM(vm);
  return (int)result;
}

//Main not used, but required. We call wren_compile directly.
int main(int argc, const char* argv[]) {
  return 0;
}

