#ifndef wren_h
#define wren_h

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

// The Wren semantic version number components.
#define WREN_VERSION_MAJOR 0
#define WREN_VERSION_MINOR 1
#define WREN_VERSION_PATCH 0

// A human-friendly string representation of the version.
#define WREN_VERSION_STRING "0.1.0"

// A monotonically increasing numeric representation of the version number. Use
// this if you want to do range checks over versions.
#define WREN_VERSION_NUMBER (WREN_VERSION_MAJOR * 1000000 + \
                             WREN_VERSION_MINOR * 1000 + \
                             WREN_VERSION_PATCH)

// A single virtual machine for executing Wren code.
//
// Wren has no global state, so all state stored by a running interpreter lives
// here.
typedef struct WrenVM WrenVM;

// A handle to a Wren object.
//
// This lets code outside of the VM hold a persistent reference to an object.
// After a handle is acquired, and until it is released, this ensures the
// garbage collector will not reclaim the object it references.
typedef struct WrenHandle WrenHandle;

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

// A finalizer function for freeing resources owned by an instance of a foreign
// class. Unlike most foreign methods, finalizers do not have access to the VM
// and should not interact with it since it's in the middle of a garbage
// collection.
typedef void (*WrenFinalizerFn)(void* data);

// Gives the host a chance to canonicalize the imported module name,
// potentially taking into account the (previously resolved) name of the module
// that contains the import. Typically, this is used to implement relative
// imports.
typedef const char* (*WrenResolveModuleFn)(WrenVM* vm,
    const char* importer, const char* name);

// Loads and returns the source code for the module [name].
typedef char* (*WrenLoadModuleFn)(WrenVM* vm, const char* name);

// Returns a pointer to a foreign method on [className] in [module] with
// [signature].
typedef WrenForeignMethodFn (*WrenBindForeignMethodFn)(WrenVM* vm,
    const char* module, const char* className, bool isStatic,
    const char* signature);

// Displays a string of text to the user.
typedef void (*WrenWriteFn)(WrenVM* vm, const char* text);

typedef enum
{
  // A syntax or resolution error detected at compile time.
  WREN_ERROR_COMPILE,

  // The error message for a runtime error.
  WREN_ERROR_RUNTIME,

  // One entry of a runtime error's stack trace.
  WREN_ERROR_STACK_TRACE
} WrenErrorType;

// Reports an error to the user.
//
// An error detected during compile time is reported by calling this once with
// [type] `WREN_ERROR_COMPILE`, the resolved name of the [module] and [line]
// where the error occurs, and the compiler's error [message].
//
// A runtime error is reported by calling this once with [type]
// `WREN_ERROR_RUNTIME`, no [module] or [line], and the runtime error's
// [message]. After that, a series of [type] `WREN_ERROR_STACK_TRACE` calls are
// made for each line in the stack trace. Each of those has the resolved
// [module] and [line] where the method or function is defined and [message] is
// the name of the method or function.
typedef void (*WrenErrorFn)(
    WrenVM* vm, WrenErrorType type, const char* module, int line,
    const char* message);

typedef struct
{
  // The callback invoked when the foreign object is created.
  //
  // This must be provided. Inside the body of this, it must call
  // [wrenSetSlotNewForeign()] exactly once.
  WrenForeignMethodFn allocate;

  // The callback invoked when the garbage collector is about to collect a
  // foreign object's memory.
  //
  // This may be `NULL` if the foreign class does not need to finalize.
  WrenFinalizerFn finalize;
} WrenForeignClassMethods;

// Returns a pair of pointers to the foreign methods used to allocate and
// finalize the data for instances of [className] in resolved [module].
typedef WrenForeignClassMethods (*WrenBindForeignClassFn)(
    WrenVM* vm, const char* module, const char* className);

typedef struct
{
  // The callback Wren will use to allocate, reallocate, and deallocate memory.
  //
  // If `NULL`, defaults to a built-in function that uses `realloc` and `free`.
  WrenReallocateFn reallocateFn;

  // The callback Wren uses to resolve a module name.
  //
  // Some host applications may wish to support "relative" imports, where the
  // meaning of an import string depends on the module that contains it. To
  // support that without baking any policy into Wren itself, the VM gives the
  // host a chance to resolve an import string.
  //
  // Before an import is loaded, it calls this, passing in the name of the
  // module that contains the import and the import string. The host app can
  // look at both of those and produce a new "canonical" string that uniquely
  // identifies the module. This string is then used as the name of the module
  // going forward. It is what is passed to [loadModuleFn], how duplicate
  // imports of the same module are detected, and how the module is reported in
  // stack traces.
  //
  // If you leave this function NULL, then the original import string is
  // treated as the resolved string.
  //
  // If an import cannot be resolved by the embedder, it should return NULL and
  // Wren will report that as a runtime error.
  //
  // Wren will take ownership of the string you return and free it for you, so
  // it should be allocated using the same allocation function you provide
  // above.
  WrenResolveModuleFn resolveModuleFn;

  // The callback Wren uses to load a module.
  //
  // Since Wren does not talk directly to the file system, it relies on the
  // embedder to physically locate and read the source code for a module. The
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

  // The callback Wren uses to report errors.
  //
  // When an error occurs, this will be called with the module name, line
  // number, and an error message. If this is `NULL`, Wren doesn't report any
  // errors.
  WrenErrorFn errorFn;

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

  // Wren will resize the heap automatically as the number of bytes
  // remaining in use after a collection changes. This number determines the
  // amount of additional memory Wren will use after a collection, as a
  // percentage of the current heap size.
  //
  // For example, say that this is 50. After a garbage collection, when there
  // are 400 bytes of memory still in use, the next collection will be triggered
  // after a total of 600 bytes are allocated (including the 400 already in
  // use.)
  //
  // Setting this to a smaller number wastes less memory, but triggers more
  // frequent garbage collections.
  //
  // If zero, defaults to 50.
  int heapGrowthPercent;

  // User-defined data associated with the VM.
  void* userData;

} WrenConfiguration;

typedef enum
{
  WREN_RESULT_SUCCESS,
  WREN_RESULT_COMPILE_ERROR,
  WREN_RESULT_RUNTIME_ERROR
} WrenInterpretResult;

// The type of an object stored in a slot.
//
// This is not necessarily the object's *class*, but instead its low level
// representation type.
typedef enum
{
  WREN_TYPE_BOOL,
  WREN_TYPE_NUM,
  WREN_TYPE_FOREIGN,
  WREN_TYPE_LIST,
WREN_TYPE_MAP,
  WREN_TYPE_NULL,
WREN_TYPE_RANGE,
  WREN_TYPE_STRING,

  // The object is of a type that isn't accessible by the C API.
  WREN_TYPE_UNKNOWN
} WrenType;

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

// Runs [source], a string of Wren source code in a new fiber in [vm] in the
// context of resolved [module].
WrenInterpretResult wrenInterpret(WrenVM* vm, const char* module,
                                  const char* source);

// Creates a handle that can be used to invoke a method with [signature] on
// using a receiver and arguments that are set up on the stack.
//
// This handle can be used repeatedly to directly invoke that method from C
// code using [wrenCall].
//
// When you are done with this handle, it must be released using
// [wrenReleaseHandle].
WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature);

// Calls [method], using the receiver and arguments previously set up on the
// stack.
//
// [method] must have been created by a call to [wrenMakeCallHandle]. The
// arguments to the method must be already on the stack. The receiver should be
// in slot 0 with the remaining arguments following it, in order. It is an
// error if the number of arguments provided does not match the method's
// signature.
//
// After this returns, you can access the return value from slot 0 on the stack.
WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method);

// Releases the reference stored in [handle]. After calling this, [handle] can
// no longer be used.
void wrenReleaseHandle(WrenVM* vm, WrenHandle* handle);

// The following functions are intended to be called from foreign methods or
// finalizers. The interface Wren provides to a foreign method is like a
// register machine: you are given a numbered array of slots that values can be
// read from and written to. Values always live in a slot (unless explicitly
// captured using wrenGetSlotHandle(), which ensures the garbage collector can
// find them.
//
// When your foreign function is called, you are given one slot for the receiver
// and each argument to the method. The receiver is in slot 0 and the arguments
// are in increasingly numbered slots after that. You are free to read and
// write to those slots as you want. If you want more slots to use as scratch
// space, you can call wrenEnsureSlots() to add more.
//
// When your function returns, every slot except slot zero is discarded and the
// value in slot zero is used as the return value of the method. If you don't
// store a return value in that slot yourself, it will retain its previous
// value, the receiver.
//
// While Wren is dynamically typed, C is not. This means the C interface has to
// support the various types of primitive values a Wren variable can hold: bool,
// double, string, etc. If we supported this for every operation in the C API,
// there would be a combinatorial explosion of functions, like "get a
// double-valued element from a list", "insert a string key and double value
// into a map", etc.
//
// To avoid that, the only way to convert to and from a raw C value is by going
// into and out of a slot. All other functions work with values already in a
// slot. So, to add an element to a list, you put the list in one slot, and the
// element in another. Then there is a single API function wrenInsertInList()
// that takes the element out of that slot and puts it into the list.
//
// The goal of this API is to be easy to use while not compromising performance.
// The latter means it does not do type or bounds checking at runtime except
// using assertions which are generally removed from release builds. C is an
// unsafe language, so it's up to you to be careful to use it correctly. In
// return, you get a very fast FFI.

// Returns the number of slots available to the current foreign method.
int wrenGetSlotCount(WrenVM* vm);

// Ensures that the foreign method stack has at least [numSlots] available for
// use, growing the stack if needed.
//
// Does not shrink the stack if it has more than enough slots.
//
// It is an error to call this from a finalizer.
void wrenEnsureSlots(WrenVM* vm, int numSlots);

// Gets the type of the object in [slot].
WrenType wrenGetSlotType(WrenVM* vm, int slot);

// Reads a boolean value from [slot].
//
// It is an error to call this if the slot does not contain a boolean value.
bool wrenGetSlotBool(WrenVM* vm, int slot);

// Reads a byte array from [slot].
//
// The memory for the returned string is owned by Wren. You can inspect it
// while in your foreign method, but cannot keep a pointer to it after the
// function returns, since the garbage collector may reclaim it.
//
// Returns a pointer to the first byte of the array and fill [length] with the
// number of bytes in the array.
//
// It is an error to call this if the slot does not contain a string.
const char* wrenGetSlotBytes(WrenVM* vm, int slot, int* length);

// Reads a number from [slot].
//
// It is an error to call this if the slot does not contain a number.
double wrenGetSlotDouble(WrenVM* vm, int slot);

// Reads a foreign object from [slot] and returns a pointer to the foreign data
// stored with it.
//
// It is an error to call this if the slot does not contain an instance of a
// foreign class.
void* wrenGetSlotForeign(WrenVM* vm, int slot);

// Reads a string from [slot].
//
// The memory for the returned string is owned by Wren. You can inspect it
// while in your foreign method, but cannot keep a pointer to it after the
// function returns, since the garbage collector may reclaim it.
//
// It is an error to call this if the slot does not contain a string.
const char* wrenGetSlotString(WrenVM* vm, int slot);

// Creates a handle for the value stored in [slot].
//
// This will prevent the object that is referred to from being garbage collected
// until the handle is released by calling [wrenReleaseHandle()].
WrenHandle* wrenGetSlotHandle(WrenVM* vm, int slot);

// Stores the boolean [value] in [slot].
void wrenSetSlotBool(WrenVM* vm, int slot, bool value);

// Stores the array [length] of [bytes] in [slot].
//
// The bytes are copied to a new string within Wren's heap, so you can free
// memory used by them after this is called.
void wrenSetSlotBytes(WrenVM* vm, int slot, const char* bytes, size_t length);

// Stores the numeric [value] in [slot].
void wrenSetSlotDouble(WrenVM* vm, int slot, double value);

// Creates a new instance of the foreign class stored in [classSlot] with [size]
// bytes of raw storage and places the resulting object in [slot].
//
// This does not invoke the foreign class's constructor on the new instance. If
// you need that to happen, call the constructor from Wren, which will then
// call the allocator foreign method. In there, call this to create the object
// and then the constructor will be invoked when the allocator returns.
//
// Returns a pointer to the foreign object's data.
void* wrenSetSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t size);

// Stores a new empty list in [slot].
void wrenSetSlotNewList(WrenVM* vm, int slot);

// Stores null in [slot].
void wrenSetSlotNull(WrenVM* vm, int slot);

// Stores the string [text] in [slot].
//
// The [text] is copied to a new string within Wren's heap, so you can free
// memory used by it after this is called. The length is calculated using
// [strlen()]. If the string may contain any null bytes in the middle, then you
// should use [wrenSetSlotBytes()] instead.
void wrenSetSlotString(WrenVM* vm, int slot, const char* text);

// Stores the value captured in [handle] in [slot].
//
// This does not release the handle for the value.
void wrenSetSlotHandle(WrenVM* vm, int slot, WrenHandle* handle);

// Returns the number of elements in the list stored in [slot].
int wrenGetListCount(WrenVM* vm, int slot);

// Reads element [index] from the list in [listSlot] and stores it in
// [elementSlot].
void wrenGetListElement(WrenVM* vm, int listSlot, int index, int elementSlot);

// Takes the value stored at [elementSlot] and inserts it into the list stored
// at [listSlot] at [index].
//
// As in Wren, negative indexes can be used to insert from the end. To append
// an element, use `-1` for the index.
void wrenInsertInList(WrenVM* vm, int listSlot, int index, int elementSlot);

// Looks up the top level variable with [name] in resolved [module] and stores
// it in [slot].
void wrenGetVariable(WrenVM* vm, const char* module, const char* name,
                     int slot);

// Sets the current fiber to be aborted, and uses the value in [slot] as the
// runtime error object.
void wrenAbortFiber(WrenVM* vm, int slot);

// Returns the user data associated with the WrenVM.
void* wrenGetUserData(WrenVM* vm);

// Sets user data associated with the WrenVM.
void wrenSetUserData(WrenVM* vm, void* userData);

// Return the type name of the object in the slot, e.g. Num, Bool, String
// Don't modify or free the string returned
const char* wrenGetSlotTypeName (WrenVM* vm, int slot);

// Retriev the type of the object in [slot] and store its class object in [classSlot].
void wrenGetSlotClass (WrenVM* vm, int slot, int classSlot);

// Checks if the object in [slot] is an instance of the type at [classSlot].
// The element in [classSlot] must be a class.
// Does not invoke any overloaded is operator.
bool wrenGetSlotIsClass (WrenVM* vm, int slot, int classSlot);

// Same as above, but take the class out of an handle instead of look for it in a slot.
// Generally faster if you can cache class objects in handles in order to reuse them.
// Also faster since it doesn't need you to reserve a slot to temporarily store the class handle to be compared.
bool wrenGetSlotIsClassHandle (WrenVM* vm, int slot, WrenHandle* classHandle);

// Checks if the value in the given slot is a bool
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotBool (WrenVM* vm, int slot);

// Checks if the value in the given slot is a number a.k.a Num / C double
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotDouble (WrenVM* vm, int slot);

// Checks if the value in the given slot is a string.
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotString (WrenVM* vm, int slot);

// Checks if the value in the given slot is a list.
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotList (WrenVM* vm, int slot);

// Checks if the value in the given slot is a map.
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotMap (WrenVM* vm, int slot);

// Checks if the value in the given slot is a range.
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotRange (WrenVM* vm, int slot);

// Checks if the value in the given slot is a foreign object, and if it is of the type in [classSlot].
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotForeign (WrenVM* vm, int slot, int classSlot);

// Checks if the value in the given slot is a foreign object, and if it is of the type passed in [classHandle].
// This is generally faster as the function above, see wrenSlotIsClass vs. wrenSlotIsClassHandle.
// If yes, true is returned
// If no, false is returned, and the current fiber is aborted with a message like "Expected parameter 2 to be of type 'Bool', got 'Num'."
bool wrenCheckSlotForeignHandle (WrenVM* vm, int slot, WrenHandle* classHandle);

// Store a new empty map in [slot].
void wrenSetSlotNewMap (WrenVM* vm, int slot);

// Retriev the value associated with a key in a map.
// mapSlot: the slot containing the map to be looked up
// keySlot: the slot containing the key too look up in the map
// valueSlot: the slot where to store the value retrieved. If you aren't actually interested in storing this value anywhere, you may pass -1 (i.e. if you just want to check the presence of the key in the map).
// If the key is present in the map, its value is tored in [valueSlot] and true is returned.
// If the key is absent from the map, null is stored in [valueSlot] and false is returned.
// This function makes the difference between null and absent: if a key is associated to null, null is stored in [valueSlot] and true is returned.
bool wrenGetMapValue (WrenVM* vm, int mapSlot, int keySlot, int valueSlot);

// Associate a value with a key in a map.
// mapSlot: the slot containing the map to be modified
// keySlot: the slot containing the key to associate
// valueSlot: the slot containing the value to associate with the key
void wrenPutInMap (WrenVM* vm, int mapSlot, int keySlot, int valueSlot);

// Remove a key/value association from a map.
// mapSlot: the slot containing the map to be modified
// keySlot: the slot containing the key to be removed from the map.
// valueSlot: the slot where to store the value that has just been removed. If you aren't interested in storing this value anywhere, you may pass -1.
// If the key was present in the map, its previous value is stored in [valueSlot], and true is returned.
// If the key wasn't present in the map, null is stored in [valueSlot] and false is returned.
// IN contrary to wrenGetMapValue above, this fonction doesn't make the difference between an absent key and a key associated to null; if a key was associated to null, false is returned.
bool wrenRemoveMap (WrenVM* vm, int mapSlot, int keySlot, int valueSlot);

// Clear the map at the given slot
void wrenClearMap (WrenVM* vm, int mapSlot);

// Store a new range in slot
void wrenSetSlotNewRange (WrenVM* vm, int slot, double from, double to, bool inclusive);

// Retriev the bounds of a range stored in the slot.
// You may pass NULL for from, to and inclusive if you aren't interested in them all.
void wrenGetSlotRange (WrenVM* vm, int slot, double* from, double* to, bool* inclusive);

// Return true if the current fiber has been aborted by a call to wrenAbortFiber or a runtime error.
bool wrenIsAborted (WrenVM* vm);

#endif
