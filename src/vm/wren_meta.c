#include "wren_meta.h"

#if WREN_USE_LIB_META

#include <string.h>

// This string literal is generated automatically from meta.wren. Do not edit.
static const char* libSource =
"class Meta {\n"
"  foreign static eval(source)\n"
"  foreign static modules\n"
"}\n";

void metaEval(WrenVM* vm)
{
  const char* source = wrenGetArgumentString(vm, 1);
  // TODO: Type check argument.
  wrenInterpret(vm, "Meta", source);
}

void metaModules(WrenVM* vm)
{
    ObjList* modules = wrenNewList(vm, 0);

    for (uint32_t i = 0; i < vm->modules->capacity; i++)
    {
        if (IS_UNDEFINED(vm->modules->entries[i].key)) continue;

        Value name = vm->modules->entries[i].key;
        if (name != NULL_VAL) // Don't add the main module file to the list.
        {
            wrenValueBufferWrite(vm, &modules->elements, name);
        }
    }

    // Set the list of module names as the return value.
    *vm->foreignCallSlot = OBJ_VAL(modules);
    vm->foreignCallSlot = NULL;
}

void wrenLoadMetaLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "", libSource);
}

WrenForeignMethodFn wrenBindMetaForeignMethod(WrenVM* vm,
                                              const char* className,
                                              const char* signature)
{
  if (strcmp(className, "Meta") != 0) return NULL;

  if (strcmp(signature, "eval(_)") == 0) return metaEval;

  if (strcmp(signature, "modules") == 0) return metaModules;

  return NULL;
}

#endif
