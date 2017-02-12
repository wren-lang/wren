^title Storing C Data

**TODO: Write these docs.**

<!--

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

-->

Until these are written, you can read the docs in [wren.h][].

[wren.h]: https://github.com/munificent/wren/blob/master/src/include/wren.h

<a class="right" href="configuring-the-vm.html">Configuring the VM &rarr;</a>
<a href="calling-c-from-wren.html">&larr; Calling C from Wren</a>
