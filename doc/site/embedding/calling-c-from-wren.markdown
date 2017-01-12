^title Calling C from Wren

- foreign class
- foreign method

- WrenBindForeignMethodFn
- finalizers (WrenFinalizerFn)
- WrenBindForeignClassFn
- WrenForeignClassMethods
- bindForeignMethodFn
- bindForeignClassFn

- wren vm reaches out to c for two things, raw data and behavior written in c
  - wren is oop so data is stored in instances of classes
    - foreign classes
  - likewise, behavior in methods, so foreign methods
- foreign methods
  - want to call code in wren, invoke method
  - want to call c code, still invoke method
  - declaring:
    - just that method is implemented in c
    - bit like "native" in java
    - need to tell wren that method is declared on class but implementation is
      in c
    - "foreign"
    - can be instance or static
  - binding:
    - wren needs to find corresponding c fn to call for it
    - uses pull model
    - when class decl is executed, wren asks embedder for pointer to c fn
    - when configure vm, give it bindForeignMethodFn
    - this is called every time foreign method decl is executed
    - tells you module, class, and sig of method
    - your job is to return proper c fn for that method
    - wren wires two together then calls c fn whenever method is called
    - looking up class and sig by name kind of slow and tedious
    - but only done once when class decl itself executes
    - after that, remembers binding
  - calling:
    - when foreign method is called, wren sets up slots
    - then calls c fn
    - receiver in slot zero, other args in later slots
    - do whatever work you want in c
    - can modify slot array
    - put return value in slot zero

**TODO: next page**

- foreign classes
  - embedded language often need to work with native c data
  - maybe want to refer to pointer to memory in c heap
  - maybe want more dense efficient encoding c provides
  - may want to refer to resource otherwise managed outside of wren, like file
    handle
  - wrap in foreign class
  - hybrid of wren and c
  - foreign class has methods defined in wren (though can be foreign)
  - each instance is instance of wren class, knows type, etc.
  - but also stores bytes of data opaque to wren
  - defining:
    - "foreign class"
    - declaring class foreign says "when construct call into c to fill in
      opaque bytes"
    - need c fn pointer to do that work
    - like method, bound when class is declared
    - bindForeignClassFn
    - takes two pointers, ctor and finalizer
  - initializing:
    - whenever construct instance of foreign class, calls fn
    - call wrenSetSlotNewForeign() to create instance and tell it how many
      bytes to allocate
    - returns void* to opaque data
    - initialize as see fit
  - accessing:
    - data opaque, can't be too opaque, need to use it!
    - cannot access from within wren, only c
    - if have instance of foreign class in slot, wrenGetSlotForeign() returns
      void* to raw data
    - typically, used if foreign method defined on foreign class
    - remember receiver in slot zero
  - freeing:
    - memory for foreign objects managed like all mem in wren
    - may be gc'd
    - usually ok
    - but foreign obj may point to resource whose lifetime should be tied to
      life of obj
    - for ex: file handle
    - if foreign obj is collected, no longer have way to get to file handle
    - if didn't close it first, leak
    - when object is gc'd, want to close file
    - define finalizer fn
    - other thing set in bindForeignClassFn
    - if provided, gc calls this before obj is collected
    - provides raw bits of obj
    - nothing else!
    - called from right inside gc, so vm is in brittle state
    - don't mess with wren, use stack, etc.
    - just free resource
    - cannot fail

Until these are written, you can read the docs in [wren.h][].

[wren.h]: https://github.com/munificent/wren/blob/master/src/include/wren.h

<a class="right" href="storing-c-data.html">Storing C Data &rarr;</a>
<a href="calling-wren-from-c.html">&larr; Calling Wren from C</a>
