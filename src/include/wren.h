#ifndef wren_h
#define wren_h

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

// A single virtual machine for executing Wren code.
//
// Wren has no global state, so all state stored by a running interpreter lives
// here.
typedef struct WrenVM WrenVM;

// A handle to a Wren object.
//
// This lets code outside of the VM hold a persistent reference to an object.
// After a value is acquired, and until it is released, this ensures the
// garbage collector will not reclaim it.
typedef struct WrenValue WrenValue;

// A generic allocation function that handles all explicit memory management
// used by Wren. It's used like so:
//
// - To allocate new memory, [memory] is NULL and [newSize] is the desired
//   size. It should return the allocated memory or NULL on failure.
//
// - To attempt to grow an existing allocation, [memory] is the memory, and
//   [newSize] is the desired size. It should return [memory] if it was able to
//   grow it in place, or a new pointer if it had to move it.
//
// - To shrink memory, [memory] and [newSize] are the same as above but it will
//   always return [memory].
//
// - To free memory, [memory] will be the memory to free and [newSize] will be
//   zero. It should return NULL.
typedef void* (*WrenReallocateFn)(void* memory, size_t newSize);

// A function callable from Wren code, but implemented in C.
typedef void (*WrenForeignMethodFn)(WrenVM* vm);

// Loads and returns the source code for the module [name].
typedef char* (*WrenLoadModuleFn)(WrenVM* vm, const char* name);

// Returns a pointer to a foreign method on [className] in [module] with
// [signature].
typedef WrenForeignMethodFn (*WrenBindForeignMethodFn)(WrenVM* vm,
                                                       const char* module,
                                                       const char* className,
                                                       bool isStatic,
                                                       const char* signature);

// Displays a string of text to the user.
typedef void (*WrenWriteFn)(WrenVM* vm, const char* text);

typedef struct
{
  // The callback invoked when the foreign object is created.
  //
  // This must be provided. Inside the body of this, it must call
  // [wrenAllocateForeign] exactly once.
  WrenForeignMethodFn allocate;

  // The callback invoked when the garbage collector is about to collecto a
  // foreign object's memory.
  //
  // This may be `NULL` if the foreign class does not need to finalize.
  WrenForeignMethodFn finalize;
} WrenForeignClassMethods;

// Returns a pair of pointers to the foreign methods used to allocate and
// finalize the data for instances of [className] in [module].
typedef WrenForeignClassMethods (*WrenBindForeignClassFn)(
    WrenVM* vm, const char* module, const char* className);

typedef struct
{
  // The callback Wren will use to allocate, reallocate, and deallocate memory.
  //
  // If `NULL`, defaults to a built-in function that uses `realloc` and `free`.
  WrenReallocateFn reallocateFn;

  // The callback Wren uses to load a module.
  //
  // Since Wren does not talk directly to the file system, it relies on the
  // embedder to phyisically locate and read the source code for a module. The
  // first time an import appears, Wren will call this and pass in the name of
  // the module being imported. The VM should return the soure code for that
  // module. Memory for the source should be allocated using [reallocateFn] and
  // Wren will take ownership over it.
  //
  // This will only be called once for any given module name. Wren caches the
  // result internally so subsequent imports of the same module will use the
  // previous source and not call this.
  //
  // If a module with the given name could not be found by the embedder, it
  // should return NULL and Wren will report that as a runtime error.
  WrenLoadModuleFn loadModuleFn;

  // The callback Wren uses to find a foreign method and bind it to a class.
  //
  // When a foreign method is declared in a class, this will be called with the
  // foreign method's module, class, and signature when the class body is
  // executed. It should return a pointer to the foreign function that will be
  // bound to that method.
  //
  // If the foreign function could not be found, this should return NULL and
  // Wren will report it as runtime error.
  WrenBindForeignMethodFn bindForeignMethodFn;

  // The callback Wren uses to find a foreign class and get its foreign methods.
  //
  // When a foreign class is declared, this will be called with the class's
  // module and name when the class body is executed. It should return the
  // foreign functions uses to allocate and (optionally) finalize the bytes
  // stored in the foreign object when an instance is created.
  WrenBindForeignClassFn bindForeignClassFn;

  // The callback Wren uses to display text when `System.print()` or the other
  // related functions are called.
  //
  // If this is `NULL`, Wren discards any printed text.
  WrenWriteFn writeFn;
  
  // The number of bytes Wren will allocate before triggering the first garbage
  // collection.
  //
  // If zero, defaults to 10MB.
  size_t initialHeapSize;

  // After a collection occurs, the threshold for the next collection is
  // determined based on the number of bytes remaining in use. This allows Wren
  // to shrink its memory usage automatically after reclaiming a large amount
  // of memory.
  //
  // This can be used to ensure that the heap does not get too small, which can
  // in turn lead to a large number of collections afterwards as the heap grows
  // back to a usable size.
  //
  // If zero, defaults to 1MB.
  size_t minHeapSize;

  // Wren will grow (and shrink) the heap automatically as the number of bytes
  // remaining in use after a collection changes. This number determines the
  // amount of additional memory Wren will use after a collection, as a
  // percentage of the current heap size.
  //
  // For example, say that this is 50. After a garbage collection, Wren there
  // are 400 bytes of memory still in use. That means the next collection will
  // be triggered after a total of 600 bytes are allocated (including the 400
  // already in use.
  //
  // Setting this to a smaller number wastes less memory, but triggers more
  // frequent garbage collections.
  //
  // If zero, defaults to 50.
  int heapGrowthPercent;
} WrenConfiguration;

typedef enum {
  WREN_RESULT_SUCCESS,
  WREN_RESULT_COMPILE_ERROR,
  WREN_RESULT_RUNTIME_ERROR
} WrenInterpretResult;

// Initializes [configuration] with all of its default values.
//
// Call this before setting the particular fields you care about.
void wrenInitConfiguration(WrenConfiguration* configuration);

// Creates a new Wren virtual machine using the given [configuration]. Wren
// will copy the configuration data, so the argument passed to this can be
// freed after calling this. If [configuration] is `NULL`, uses a default
// configuration.
WrenVM* wrenNewVM(WrenConfiguration* configuration);

// Disposes of all resources is use by [vm], which was previously created by a
// call to [wrenNewVM].
void wrenFreeVM(WrenVM* vm);

// Immediately run the garbage collector to free unused memory.
void wrenCollectGarbage(WrenVM* vm);

// Runs [source], a string of Wren source code in a new fiber in [vm].
WrenInterpretResult wrenInterpret(WrenVM* vm, const char* source);

// Creates a handle that can be used to invoke a method with [signature] on the
// object in [module] currently stored in top-level [variable].
//
// This handle can be used repeatedly to directly invoke that method from C
// code using [wrenCall].
//
// When done with this handle, it must be released using [wrenReleaseValue].
WrenValue* wrenGetMethod(WrenVM* vm, const char* module, const char* variable,
                         const char* signature);

// Calls [method], passing in a series of arguments whose types must match the
// specifed [argTypes]. This is a string where each character identifies the
// type of a single argument, in order. The allowed types are:
//
// - "b" - A C `int` converted to a Wren Bool.
// - "d" - A C `double` converted to a Wren Num.
// - "i" - A C `int` converted to a Wren Num.
// - "s" - A C null-terminated `const char*` converted to a Wren String. Wren
//         will allocate its own string and copy the characters from this, so
//         you don't have to worry about the lifetime of the string you pass to
//         Wren.
// - "a" - An array of bytes converted to a Wren String. This requires two
//         consecutive arguments in the argument list: `const char*` pointing
//         to the array of bytes, followed by an `int` defining the length of
//         the array. This is used when the passed string may contain null
//         bytes, or just to avoid the implicit `strlen()` call of "s" if you
//         happen to already know the length.
// - "v" - A previously acquired WrenValue*. Passing this in does not implicitly
//         release the value. If the passed argument is NULL, this becomes a
//         Wren NULL.
//
// [method] must have been created by a call to [wrenGetMethod]. If
// [returnValue] is not `NULL`, the return value of the method will be stored
// in a new [WrenValue] that [returnValue] will point to. Don't forget to
// release it, when done with it.
WrenInterpretResult wrenCall(WrenVM* vm, WrenValue* method,
                             WrenValue** returnValue,
                             const char* argTypes, ...);

WrenInterpretResult wrenCallVarArgs(WrenVM* vm, WrenValue* method,
                                    WrenValue** returnValue,
                                    const char* argTypes, va_list args);

// Releases the reference stored in [value]. After calling this, [value] can no
// longer be used.
void wrenReleaseValue(WrenVM* vm, WrenValue* value);

// This must be called once inside a foreign class's allocator function.
//
// It tells Wren how many bytes of raw data need to be stored in the foreign
// object and creates the new object with that size. It returns a pointer to
// the foreign object's data.
void* wrenAllocateForeign(WrenVM* vm, size_t size);

// Returns the number of arguments available to the current foreign method.
int wrenGetArgumentCount(WrenVM* vm);

// The following functions read one of the arguments passed to a foreign call.
// They may only be called while within a function provided to
// [wrenDefineMethod] or [wrenDefineStaticMethod] that Wren has invoked.
//
// They retreive the argument at a given index which ranges from 0 to the number
// of parameters the method expects. The zeroth parameter is used for the
// receiver of the method. For example, given a foreign method "foo" on String
// invoked like:
//
//     "receiver".foo("one", "two", "three")
//
// The foreign function will be able to access the arguments like so:
//
//     0: "receiver"
//     1: "one"
//     2: "two"
//     3: "three"
//
// It is an error to pass an invalid argument index.

// Reads a boolean argument for a foreign call. Returns false if the argument
// is not a boolean.
bool wrenGetArgumentBool(WrenVM* vm, int index);

// Reads a numeric argument for a foreign call. Returns 0 if the argument is not
// a number.
double wrenGetArgumentDouble(WrenVM* vm, int index);

// Reads a foreign object argument for a foreign call and returns a pointer to
// the foreign data stored with it. Returns NULL if the argument is not a
// foreign object.
void* wrenGetArgumentForeign(WrenVM* vm, int index);

// Reads an string argument for a foreign call. Returns NULL if the argument is
// not a string.
//
// The memory for the returned string is owned by Wren. You can inspect it
// while in your foreign function, but cannot keep a pointer to it after the
// function returns, since the garbage collector may reclaim it.
const char* wrenGetArgumentString(WrenVM* vm, int index);

// Creates a handle for the value passed as an argument to a foreign call.
//
// This will prevent the object that is referred to from being garbage collected
// until the handle is released by calling [wrenReleaseValue()].
WrenValue* wrenGetArgumentValue(WrenVM* vm, int index);

// The following functions provide the return value for a foreign method back
// to Wren. Like above, they may only be called during a foreign call invoked
// by Wren.
//
// If none of these is called by the time the foreign function returns, the
// method implicitly returns `null`. Within a given foreign call, you may only
// call one of these once. It is an error to access any of the foreign calls
// arguments after one of these has been called.

// Provides a boolean return value for a foreign call.
void wrenReturnBool(WrenVM* vm, bool value);

// Provides a numeric return value for a foreign call.
void wrenReturnDouble(WrenVM* vm, double value);

// Provides a string return value for a foreign call.
//
// The [text] will be copied to a new string within Wren's heap, so you can
// free memory used by it after this is called.
//
// If [length] is non-zero, Wren copies that many bytes from [text], including
// any null bytes. If it is -1, then the length of [text] is calculated using
// `strlen()`. If the string may contain any null bytes in the middle, then you
// must pass an explicit length.
void wrenReturnString(WrenVM* vm, const char* text, int length);

// Provides the return value for a foreign call.
//
// This uses the value referred to by the handle as the return value, but it
// does not release the handle.
void wrenReturnValue(WrenVM* vm, WrenValue* value);

#endif
