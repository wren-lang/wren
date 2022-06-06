#pragma once
#ifndef wren_debugger_types_h
#define wren_debugger_types_h

#include "wren_common.h"
#include "wren_utils.h"

#if WREN_DEBUGGER

DECLARE_BUFFER(SymbolTable, SymbolTable);
DECLARE_BUFFER(IntBuffer, IntBuffer);

typedef struct {

    //The locals table stores an index for each local variable in the function
    //Indexes correspond to entries in [localIndixes], which give the index into the function stack
  SymbolTable locals;
  IntBuffer localIndexes;
  //:todo: an int buffer is wasteful
  IntBuffer isUpvalue;
  IntBuffer startLines; //When a local was defined
  IntBuffer endLines; //Last line that a local was defined on

} FnDebugLocals;

typedef struct {
    // Maps class name to an index into [fieldIndices] and [fieldSlots]
  SymbolTable classIndices;
  // For each class in this module, stores the class' field name -> index into fieldSlots
  SymbolTableBuffer fieldIndices;
  // For each field class in this module, stores the actual slot index of the given field
  IntBufferBuffer fieldSlots;
} ObjModuleDebug;


#endif //WREN_DEBUGGER
#endif //wren_debugger_types_h