^title Sequence Class
^category core

An abstract base class for any iterable object. Any class that implements the
core [iterator protocol][] can extend this to get a number of helpful methods.

[iterator protocol]: ../control-flow.html#the-iterator-protocol

### **all**(predicate)

Tests whether all the elements in the sequence pass the `predicate`.

Iterates over the sequence, passing each element to the function `predicate`.
If it returns `false`, stops iterating and returns `false`. Otherwise, returns
`true`.

    :::dart
    [1, 2, 3].all {|n| n > 2} // False.
    [1, 2, 3].all {|n| n < 4} // True.

### **join**(sep)

Returns a string representation of the list. The string representations of the
elements in the list is concatenated with intervening occurrences of `sep`.

It is a runtime error if `sep` is not a string.

### **join**

Calls `join` with the empty string as the separator.

### **map**(transformation)

Creates a new list by applying `transformation` to each element in the
sequence.

Iterates over the sequence, passing each element to the function
`transformation`. Generates a new list from the result of each of those calls.

    :::dart
    [1, 2, 3].map {|n| n * 2} // [2, 4, 6].

### **reduce**(function)

Reduces the sequence down to a single value. `function` is a function that takes two arguments, the accumulator and sequence item and returns the new accumulator value. The accumulator is initialized from the first item in the sequence. Then, the function is invoked on each remaining item in the sequence, iteratively updating the accumulator.

It is a runtime error to call this on an empty sequence.

### **reduce**(seed, function)

Similar to above, but uses `seed` for the initial value of the accumulator. If the sequence is empty, returns `seed`.

### **where**(predicate)

Produces a new list containing only the elements in the sequence that pass the
`predicate`.

Iterates over the sequence, passing each element to the function `predicate`.
If it returns `true`, adds the element to the result list.

    (1..10).where {|n| n % 2 == 1} // [1, 3, 5, 7, 9].
