#ifndef wren_parser_h
#define wren_parser_h

#include "wren_vm.h"

// This module defines the compiler for Wren. It takes a string of source code
// and lexes, parses, and compiles it. Wren uses a single-pass compiler. It
// does not build an actual AST during parsing and then consume that to
// generate code. Instead, the parser directly emits bytecode.
//
// This forces a few restrictions on the grammar and semantics of the language.
// Things like forward references and arbitrary lookahead are much harder. We
// get a lot in return for that, though.
//
// The implementation is much simpler since we don't need to define a bunch of
// AST data structures. More so, we don't have to deal with managing memory for
// AST objects. The compiler does almost no dynamic allocation while running.
//
// Compilation is also faster since we don't create a bunch of temporary data
// structures and destroy.

// Compiles [source], a string of Wren source code, to an [ObjFn] that will
// execute that code when invoked.
ObjFn* wrenCompile(WrenVM* vm, const char* source);

#endif
