#ifndef wren_parser_h
#define wren_parser_h

#include "vm.h"

ObjFn* wrenCompile(WrenVM* vm, const char* source);

#endif
