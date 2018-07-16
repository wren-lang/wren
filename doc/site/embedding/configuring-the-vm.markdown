^title Configuring the VM

When you create a Wren VM, you tweak it by passing in a pointer to a
WrenConfiguration structure. Since Wren has no global state, you can configure
each VM differently if your application happens to run multiple.

The struct looks like:

    :::c
    typedef struct
    {
      WrenReallocateFn reallocateFn;
      WrenLoadModuleFn loadModuleFn;
      WrenBindForeignMethodFn bindForeignMethodFn;
      WrenBindForeignClassFn bindForeignClassFn;
      WrenWriteFn writeFn;
      WrenErrorFn errorFn;
      size_t initialHeapSize;
      size_t minHeapSize;
      int heapGrowthPercent;
    } WrenConfiguration;

Most fields have useful defaults, which you can (and should) initialize by
calling:

    :::c
    wrenInitConfiguration(&configuration);

Calling this ensures that your VM doesn't get uninitialized configuration when
new fields are added to WrenConfiguration. Here is what each field does, roughly
categorized:

## Binding

The VM is isolated from the outside world. These callbacks let the VM request
access to imported code and foreign functionality.

### `loadModuleFn`

This is the callback Wren uses to load an imported module. The VM itself does
not know how to talk to the file system, so when an `import` statement is
executed, it relies on the host application to locate and read the source code
for a module.

The signature of this function is:

    :::c
    char* loadModule(WrenVM* vm, const char* name)

When a module is imported, Wren calls this and passes in the module's name. The
host should return the source code for that module. Memory for the source should
be allocated using the same allocator that the VM uses for other allocation (see
below). Wren will take ownership of the returned string and free it later.

The module loader is only be called once for any given module name. Wren caches
the result internally so subsequent imports of the same module use the
previously loaded code.

If your host application isn't able to load a module with some name, it should
return `NULL` and Wren will report that as a runtime error.

If you don't use any `import` statements, you can leave this `NULL`.

### `bindForeignMethodFn`

The callback Wren uses to find a foreign method and bind it to a class. See
[this page][foreign method] for details. If your application defines no foreign
methods, you can leave this `NULL`.

[foreign method]: /embedding/calling-c-from-wren.html

### `bindForeignClassFn`

The callback Wren uses to find a foreign class and get its foreign methods. See
[this page][foreign class] for details. If your application defines no foreign
classes, you can leave this `NULL`.

[foreign class]: /embedding/storing-c-data.html

## Diagnostics

These let you wire up some minimal output so you can tell if your code is doing
what you expect.

### `writeFn`

This is the callback Wren uses to output text when `System.print()` or the other
related functions are called. This is the minimal connection the VM has with the
outside world and lets you do rudimentary "printf debugging". Its signature is:

    :::c
    void write(WrenVM* vm, const char* text)

Wren does *not* have a default implementation for this. It's up to you to wire
it up to `printf()` or some other way to show the text. If you leave it `NULL`,
calls to `System.print()` and others silently do nothing.

### `errorFn`

Wren uses this callback to report compile time and runtime errors. Its signature
is:

    :::c
    void error(
          WrenVM* vm, 
          WrenErrorType type,
          const char* module,
          int line,
          const char* message)

The `type` parameter is one of:

    :::c
    typedef enum
    {
      // A syntax or resolution error detected at compile time.
      WREN_ERROR_COMPILE,

      // The error message for a runtime error.
      WREN_ERROR_RUNTIME,

      // One entry of a runtime error's stack trace.
      WREN_ERROR_STACK_TRACE
    } WrenErrorType;

When a compile error occurs, `errorFn` is called once with type
`WREN_ERROR_COMPILE`, the name of the module and line where the error occurs,
and the error message.

Runtime errors include stack traces. To handle this, Wren first calls `errorFn`
with `WREN_ERROR_RUNTIME`, no module or line, and the runtime error's message.
After that, it calls `errorFn` again using type `WREN_ERROR_STACK_TRACE`, once
for each line in the stack trace. Each of those calls has the module and line
where the method or function is defined and `message` is the name of the method
or function.

If you leave this `NULL`, Wren does not report any errors.

## Memory Management

These fields control how the VM allocates and manages memory.

### `reallocateFn`

This lets you provide a custom memory allocation function. Its signature is:

    :::c
    void* reallocate(void* memory, size_t newSize)

Wren uses this one function to allocate, grow, shrink, and deallocate memory.
When called, `memory` is the existing pointer to the block of memory if an
allocation is being changed or freed. If Wren is requesting new memory, then
`memory` is `NULL`.

`newSize` is the number of bytes of memory being requested. If memory is being
freed, this is zero. Your callback should allocate the proper amount of memory
and return it.

If you don't provide a custom allocator, the VM uses a default one that relies
on `realloc` and `free`.

### `initialHeapSize`

This defines the total number of bytes of memory the VM will allocate before
triggering the first garbage collection. Setting this to a smaller number
reduces the amount of memory Wren will have allocated at one time, but causes it
to collect garbage more frequently.

If you set this to zero, Wren uses a default size of 10MB.

### `minHeapSize`

After a garbage collection occurs, the threshold for the *next* collection is
determined based on the number of bytes remaining in use. This allows Wren to
grow or shrink its memory usage automatically based on how much memory is
actually needed.

This can be used to ensure that the heap does not get *too* small, which can
in turn lead to a large number of collections afterwards as the heap grows
back to a usable size.

If zero, this defaults to 1MB.

### `heapGrowthPercent`

Wren tunes the rate of garbage collection based on how much memory is still in
use after a collection. This number controls that. It determines the amount of
additional memory Wren will use after a collection, as a percentage of the
current heap size.

For example, say that this is 50. After a garbage collection, there are 400
bytes of memory still in use. That means the next collection will be triggered
after a total of 600 bytes are allocated (including the 400 already in use.)

Setting this to a smaller number wastes less memory, but triggers more
frequent garbage collections.

If set to zero, the VM uses a default of 50.

<a href="storing-c-data.html">&larr; Storing C Data</a>
