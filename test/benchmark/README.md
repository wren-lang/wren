The benchmarks in here attempt to faithfully implement the exact same algorithm in a few different languages. We're using Lua, Python, and Ruby for comparison here because those are all in Wren's ballpark: dynamically-typed, object-oriented, bytecode-compiled.

A bit about each benchmark:

### binary_trees

This benchmark stresses object creation and garbage collection. It builds a few big, deeply nested binaries and then traverses them.

### fib

This is just a simple naïve Fibonacci number calculator. It was the first benchmark I wrote when Wren supported little more than function calls and arithmetic. It isn't particularly representative of real-world code, but it does stress function call and arithmetic.

### for

This microbenchmark just tests the performance of for loops. Not too useful, but i used it when implementing `for` in Wren to make sure it wasn't too far off the mark.

### method_call

This is the most useful benchmark: it tests dynamic dispatch and polymorphism. You'll note that the main iteration loop is unrolled in all of the implementations. This is to ensure that the loop overhead itself doesn't dwarf the method call time.
