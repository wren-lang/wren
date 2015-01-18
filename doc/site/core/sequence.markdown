^title Sequence Class
^category core

An abstract base class for any iterable object. It provides a number of methods for working with sequences based on the core [iterator protocol](../control-flow.html#the-iterator-protocol).

### **all**(predicate)

Tests whether all the elements in the list pass the `predicate`.

### **reduce**(function)

Reduces the sequence down to a single value. `function` is a function that takes two arguments, the accumulator and sequence item and returns the new accumulator value. The accumulator is initialized from the first item in the sequence. Then, the function is invoked on each remaining item in the sequence, iteratively updating the accumulator.

It is a runtime error to call this on an empty sequence.

### **reduce**(seed, function)

Similar to above, but uses `seed` for the initial value of the accumulator. If the sequence is empty, returns `seed`.
