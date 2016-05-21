^title Embedding

Wren is designed to be a scripting language that lives inside a host
application, so the embedding API is as important as any of its language
features. Designing this API well requires satisfying several constraints:

1. **Wren is dynamically typed, but C is not.** A variable can hold a value of
   any type in Wren, but that's definitely not the case in C unless you define
   some sort of variant type, which ultimately just kicks the problem down the
   road. Eventually, we have to move data across the boundary between typed and
   untyped code.

2. **Wren uses garbage collection, but C manages memory manually.** GC adds a
   few constraints on the API. The VM must be able to find every Wren object
   that is still usable, even if that object is being held onto by native C
   code. Otherwise, it could free an object that's still in use.

    Also, we ideally don't want to let native C code see a bare pointer to a
    chunk of memory managed by Wren. Many garbage collection strategies involve
    [moving objects][] in memory. If we allow C code to point directly to an
    object, that pointer will be left dangling when the object moves. To prevent
    that, we can't using moving GCs even though they have some nice performance
    properties.

3. **The embedding API needs to be fast.** Users may add layers of abstraction
   on top of the API to make it more pleasant to work with, but the base API
   defines the *maximum* performance you can get out of the system. It's the
   bottom of the stack, so there's no way for a user to optimize around it if
   it's too slow. There is no lower level alternative.

4. **We want the API to be pleasant to use.** This is the last constraint
   because it's the softest. Of course, we want a beautiful, usable, API. But we
   really *need* to handle the above, so we're willing to make things a bit more
   of a chore to reach the first three goals.

[moving objects]: https://en.wikipedia.org/wiki/Tracing_garbage_collection#Copying_vs._mark-and-sweep_vs._mark-and-don.27t-sweep

Fortunately, Wren isn't the first language to tackle this. If you're familiar
with [Lua's C API][lua], you'll find Wren's to be fairly familiar.

[lua]: https://www.lua.org/pil/24.html

### Performance and safety

When code is safely snuggled within the confines of the VM, it's pretty safe.
Method calls are dynamically checked and generate a runtime error which can be
caught and handled. The stack grows if it gets close to overflowing. In general,
when you're within Wren code, it tries very hard to avoid crashing and burning.

This is why you use a high level language after all&mdash;it's safer and more
productive than C. C, meanwhile, really assumes you know what you're doing. You
can cast pointers in invalid ways, misinterpret bits, use memory after freeing
it, etc. What that gives you in return is blazing performance. Many of the
reasons C is fast are because it takes all the governors and guardrails off.

Wren's embedding API defines the border between those worlds, and takes on some
of the characteristics of C. When you call any of the embedding API functions,
it assumes you are calling them correctly. If you invoke a Wren method from C
that expects three arguments, it trusts that you gave it three arguments.

In debug builds, Wren has assertions to check as many things as it can, but in
release builds, Wren expects you to do the right thing. This means you need to
take care when using the embedding API, just like you do in all C code you
write. In return, you get an API that is quite fast.

## Including Wren

There are two (well, three) ways to get the Wren VM into your program:

1.  **Link to the static or dynamic library.** When you [build Wren][build], it
    generates both shared and static libraries in `lib` that you can link to.

2.  **Include the source directly in your application.** If you want to include
    the source directly in your program, you don't need to run any build steps.
    Just add the source files in `src/vm` to your project. They should compile
    cleanly as C99 or C++98 or anything later.

[build]: ../getting-started.html

In either case, you also want to add `src/include` to your include path so you
can get to the [public header for Wren][wren.h]:

[wren.h]: https://github.com/munificent/wren/blob/master/src/include/wren.h

    :::c
    #include "wren.h"

If your program is in C++ but you are linking to the Wren library compiled as C,
then you'll need to handle the calling convention differences like so:

    :::c
    extern "C" {
    #include "wren.h"
    }

(Wren's source can be compiled as either C or C++, so you can avoid this by
compiling the whole thing yourself as C++. Whatever floats your boat.)

## Creating a Wren VM

Once you've integrated the code into your executable, you need to create a
virtual machine. To do that, you create a WrenConfiguration:

    :::c
    WrenConfiguration config;
    wrenInitConfiguration(&config);

This gives you a basic configuration that has reasonable defaults for
everything. If you don't need to tweak stuff, you can leave it at that. We'll
[learn more][configuration] about what you can configure later.

[configuration]: configuration.html

With this ready, you can create the VM:

    :::c
    WrenVM* vm = wrenNewVM(&config);

This allocates memory for a new VM and initializes it. The Wren C implementation
has no global state, so every single bit of data Wren uses is bundled up inside
a WrenVM. You can have multiple Wren VMs running independently of each other
without any problems.

`wrenNewVM()` stores its own copy of the configuration, so after calling it, you
can discard the `WrenConfiguration` struct you filled in. Now you have a live
VM, waiting to run some code!

## Executing Wren code

You can tell the VM to execute a string of Wren source code like so:

    :::c
    WrenInterpretResult result = wrenInterpret(vm,
        "System.print(\"I am running in a VM!\")");

The string is a chunk of Wren code to execute&mdash;a series of one or more
statements separated by newlines. Wren copies the string, so you can free it
after calling this.

When you call `wrenInterpret()`, Wren first compiles your source to bytecode. If
an error occurs, it returns immediately with `WREN_RESULT_COMPILE_ERROR`.

Otherwise, Wren spins up a new [fiber][] and executes the code in that. Your
code can in turn spawn whatever other fibers it wants. It keeps running fibers
until they all complete or the current one [suspends].

[fiber]: ../concurrency.html
[suspends]: ../modules/core/fiber.html#fiber.suspend()

The code runs in a special "main" module. Everytime you call `wrenInterpret()`,
it runs in the same module. That way, top-level names defined in one call can be
accessed in later ones.

If a [runtime error][] occurs (and another fiber doesn't handle it), Wren abort
fibers all the way back to the main one and returns `WREN_RESULT_RUNTIME_ERROR`.
Otherwise, when the last fiber successfully returns, it returns
`WREN_RESULT_SUCCESS`.

[runtime error]: error-handling.html

## Shutting down a VM

Once the party is over and you're ready to end your relationship with a VM, you
need to free any memory it allocated. You do that like so:

    :::c
    wrenFreeVM(vm);

After calling that, you obviously cannot use the `WrenVM*` you passed to it
again. It's dead.

Note that Wren will yell at you if you still have any live [WrenHandle][handle]
objects when you call this. This makes sure you haven't lost track of any of
them (which leaks memory) and you don't try to use any of them after the VM has
been freed.

[handle]: slots-and-handles.html#handles

Next, we'll learn to make that VM do useful stuff...

<a class="right" href="slots-and-handles.html">Slots and Handles &rarr;</a>
