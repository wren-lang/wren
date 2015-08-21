^title Embedding API
^category reference

Wren is designed to be a scripting language, so the embedding API is as
important as any of its language features. There are two (well, three) ways to get Wren into
your application:

1.  **Link to static or dynamic library.** When you [build Wren][build], it
    generates both shared and static libraries in `lib` that you can link to.

2.  **Include the source directly in your application.** If you want to include
    the source directly in your program, you don't need to run any build steps.
    Just add the source files in `src/vm` to your project. They should compile
    cleanly as C99 or C++89 or anything later.

[build]: getting-started.html

You also want to add `src/include` to your include path so you can get to the
[public header for Wren][wren.h]:

[wren.h]: https://github.com/munificent/wren/blob/master/src/include/wren.h

    :::c
    #include "wren.h"

## Hosting a Wren VM

Once you've integrated the code into your executable, you need to create a
virtual machine. To do that, you first fill in a `WrenConfiguration`:

    :::c
    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.reallocateFn = ...;
    config.loadModuleFn = ...;
    config.bindForeignMethodFn = ...;
    config.bindForeignClassFn = ...;
    config.initialHeapSize = ...;
    config.minHeapSize = ...;
    config.heapGrowthPercent = ...;

The `reallocateFn` is a callback you can provide to control how Wren allocates
and frees memory. If you leave that to the default, it uses `malloc()` and
`free()`.

The next three callbacks are how Wren talks back to your program. We'll cover
those in detail later. Then there a few fields to let you tune how the garbage
collector runs. You can tweak these if you want, but the defaults are usually
fine.

Now you can create a VM:

    :::c
    WrenVM* vm = wrenNewVM(&config);

This allocates memory for a new VM using the same `reallocateFn` you provided.
The Wren C implementation has no global state, so every single bit of data Wren
uses is bundled up inside a `WrenVM`. You can have multiple Wren VMs running
independently from each other without any problems.

`wrenNewVM()` stores its own copy of the configuration, so after calling it, you
can discard the `WrenConfiguration` struct you filled in. Now you have a live
VM, waiting to run some code!

## Executing Wren code

You can tell the VM to execute a string of Wren source code like so:

    :::c
    WrenInterpretResult result = wrenInterpret(vm,
        "<where>",
        "IO.print(\"Hi!\")");

The first string parameter is a "source path". It's just an arbitrary string that describes where the source code is from. It's what shows up in stack traces if a runtime error occurs in the code. It can be whatever you want as long as it's not `NULL`.

The other string is the chunk of code to execute—a series of one or more
statements separated by newlines. Wren runs this code in a special "main"
module. Each time you call this, the code is run in the same module. This way,
top-level names defined in one call can be accessed in later ones.

When you call `wrenInterpret()`, Wren first compiles your source to bytecode. If an error occurs here, it returns immediately with `WREN_RESULT_COMPILE_ERROR`. Otherwise, Wren spins up a new [fiber][] and executes the code in that. Your code can in turn spawn whatever other fibers it wants. It keeps running fibers until they all complete.

[fiber]: fibers.html

If a [runtime error][] occurs (and another fiber doesn't catch it), it will abort fibers all the way back to the main one and then return `WREN_RESULT_RUNTIME_ERROR`. Otherwise, when the last fiber successfully returns, it returns `WREN_RESULT_SUCCESS`.

[runtime error]: error-handling.html

## Calling a C function from Wren

**TODO**

## Calling a Wren method from C

**TODO**

## Storing a reference to a Wren object in C

**TODO**

## Storing C data in a Wren object

**TODO**

## Shutting down a VM

Once the party is over and you're ready to end your relationship with a VM, you need to free any memory it allocated. You do that like so:

    :::c
    wrenFreeVM(vm);

After calling that, you obviously cannot use the `WrenVM*` you passed to it again. It's dead.

Note that Wren will yell at you if you still have any live `WrenValue` or `WrenMethod` objects when you call this. This makes sure you haven't lost track of any of them (which leaks memory) and you don't try to use any of them after the VM has been freed.
