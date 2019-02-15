#ifndef wren_compiler_h
#define wren_compiler_h

#include "wren.h"
#include "wren_value.h"

typedef struct sCompiler Compiler;

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
// structures and destroy them after generating code.

// Compiles [source], a string of Wren source code located in [module], to an
// [ObjFn] that will execute that code when invoked. Returns `NULL` if the
// source contains any syntax errors.
//
// If [isExpression] is `true`, [source] should be a single expression, and
// this compiles it to a function that evaluates and returns that expression.
// Otherwise, [source] should be a series of top level statements.
//
// If [printErrors] is `true`, any compile errors are output to stderr.
// Otherwise, they are silently discarded.
ObjFn* wrenCompile(WrenVM* vm, ObjModule* module, const char* source,
                   bool isExpression, bool printErrors);

// When a class is defined, its superclass is not known until runtime since
// class definitions are just imperative statements. Most of the bytecode for a
// a method doesn't care, but there are two places where it matters:
//
//   - To load or store a field, we need to know the index of the field in the
//     instance's field array. We need to adjust this so that subclass fields
//     are positioned after superclass fields, and we don't know this until the
//     superclass is known.
//
//   - Superclass calls need to know which superclass to dispatch to.
//
// We could handle this dynamically, but that adds overhead. Instead, when a
// method is bound, we walk the bytecode for the function and patch it up.
void wrenBindMethodCode(ObjClass* classObj, ObjFn* fn);

// Reaches all of the heap-allocated objects in use by [compiler] (and all of
// its parents) so that they are not collected by the GC.
void wrenMarkCompiler(WrenVM* vm, Compiler* compiler);

#endif
