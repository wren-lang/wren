// This defines the bytecode instructions used by the VM. It does so by invoking
// an OPCODE() macro which is expected to be defined at the point that this is
// included. See: http://en.wikipedia.org/wiki/X_Macro.
//
// Note that the order of instructions here affects the order of the dispatch
// table in the VM's interpreter loop. That in turn affects caching which
// affects overall performance. Take care to run benchmarks if you change the
// order here.

// Load the constant at index [arg].
OPCODE(CONSTANT)

// Push null onto the stack.
OPCODE(NULL)

// Push false onto the stack.
OPCODE(FALSE)

// Push true onto the stack.
OPCODE(TRUE)

// Pushes the value in the given local slot.
OPCODE(LOAD_LOCAL_0)
OPCODE(LOAD_LOCAL_1)
OPCODE(LOAD_LOCAL_2)
OPCODE(LOAD_LOCAL_3)
OPCODE(LOAD_LOCAL_4)
OPCODE(LOAD_LOCAL_5)
OPCODE(LOAD_LOCAL_6)
OPCODE(LOAD_LOCAL_7)
OPCODE(LOAD_LOCAL_8)

// Note: The compiler assumes the following _STORE instructions always
// immediately follow their corresponding _LOAD ones.

// Pushes the value in local slot [arg].
OPCODE(LOAD_LOCAL)

// Stores the top of stack in local slot [arg]. Does not pop it.
OPCODE(STORE_LOCAL)

// Pushes the value in upvalue [arg].
OPCODE(LOAD_UPVALUE)

// Stores the top of stack in upvalue [arg]. Does not pop it.
OPCODE(STORE_UPVALUE)

// Pushes the value of the top-level variable in slot [arg].
OPCODE(LOAD_MODULE_VAR)

// Stores the top of stack in top-level variable slot [arg]. Does not pop it.
OPCODE(STORE_MODULE_VAR)

// Pushes the value of the field in slot [arg] of the receiver of the current
// function. This is used for regular field accesses on "this" directly in
// methods. This instruction is faster than the more general CODE_LOAD_FIELD
// instruction.
OPCODE(LOAD_FIELD_THIS)

// Stores the top of the stack in field slot [arg] in the receiver of the
// current value. Does not pop the value. This instruction is faster than the
// more general CODE_LOAD_FIELD instruction.
OPCODE(STORE_FIELD_THIS)

// Pops an instance and pushes the value of the field in slot [arg] of it.
OPCODE(LOAD_FIELD)

// Pops an instance and stores the subsequent top of stack in field slot
// [arg] in it. Does not pop the value.
OPCODE(STORE_FIELD)

// Pop and discard the top of stack.
OPCODE(POP)

// Push a copy of the value currently on the top of the stack.
OPCODE(DUP)

// Invoke the method with symbol [arg]. The number indicates the number of
// arguments (not including the receiver).
OPCODE(CALL_0)
OPCODE(CALL_1)
OPCODE(CALL_2)
OPCODE(CALL_3)
OPCODE(CALL_4)
OPCODE(CALL_5)
OPCODE(CALL_6)
OPCODE(CALL_7)
OPCODE(CALL_8)
OPCODE(CALL_9)
OPCODE(CALL_10)
OPCODE(CALL_11)
OPCODE(CALL_12)
OPCODE(CALL_13)
OPCODE(CALL_14)
OPCODE(CALL_15)
OPCODE(CALL_16)

// Invoke a superclass method with symbol [arg]. The number indicates the
// number of arguments (not including the receiver).
OPCODE(SUPER_0)
OPCODE(SUPER_1)
OPCODE(SUPER_2)
OPCODE(SUPER_3)
OPCODE(SUPER_4)
OPCODE(SUPER_5)
OPCODE(SUPER_6)
OPCODE(SUPER_7)
OPCODE(SUPER_8)
OPCODE(SUPER_9)
OPCODE(SUPER_10)
OPCODE(SUPER_11)
OPCODE(SUPER_12)
OPCODE(SUPER_13)
OPCODE(SUPER_14)
OPCODE(SUPER_15)
OPCODE(SUPER_16)

// Jump the instruction pointer [arg] bytes forward or backward.
OPCODE(JUMP)

// Pop and if not truthy then jump the instruction pointer [arg] bytes forward or backward.
OPCODE(JUMP_IF)

// If the top of the stack is false, jump [arg] bytes forward or backward.
// Otherwise, pop and continue.
OPCODE(AND)

// If the top of the stack is non-false, jump [arg] bytes forward or backward.
// Otherwise, pop and continue.
OPCODE(OR)

// Pop [a] then [b] and push true if [b] is an instance of [a].
OPCODE(IS)

// Close the upvalue for the local on the top of the stack, then pop it.
OPCODE(CLOSE_UPVALUE)

// Exit from the current function and return the value on the top of the
// stack.
OPCODE(RETURN)

// Creates a closure for the function stored at [arg] in the constant table.
//
// Following the function argument is a number of arguments, two for each
// upvalue. The first is true if the variable being captured is a local (as
// opposed to an upvalue), and the second is the index of the local or
// upvalue being captured.
//
// Pushes the created closure.
OPCODE(CLOSURE)

// Creates a class. Top of stack is the superclass, or `null` if the class
// inherits Object. Below that is a string for the name of the class. Byte
// [arg] is the number of fields in the class.
OPCODE(CLASS)

// Define a method for symbol [arg]. The class receiving the method is popped
// off the stack, then the function defining the body is popped.
//
// If a foreign method is being defined, the "function" will be a string
// identifying the foreign method. Otherwise, it will be a function or
// closure.
OPCODE(METHOD_INSTANCE)

// Define a method for symbol [arg]. The class whose metaclass will receive
// the method is popped off the stack, then the function defining the body is
// popped.
//
// If a foreign method is being defined, the "function" will be a string
// identifying the foreign method. Otherwise, it will be a function or
// closure.
OPCODE(METHOD_STATIC)

// Load the module whose name is stored in string constant [arg]. Pushes
// NULL onto the stack. If the module has already been loaded, does nothing
// else. Otherwise, it creates a fiber to run the desired module and switches
// to that. When that fiber is done, the current one is resumed.
OPCODE(LOAD_MODULE)

// Reads a top-level variable from another module. [arg1] is a string
// constant for the name of the module, and [arg2] is a string constant for
// the variable name. Pushes the variable if found, or generates a runtime
// error otherwise.
OPCODE(IMPORT_VARIABLE)

// This pseudo-instruction indicates the end of the bytecode. It should
// always be preceded by a `CODE_RETURN`, so is never actually executed.
OPCODE(END)
