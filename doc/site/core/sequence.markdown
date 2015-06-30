^title Sequence Class
^category core

An abstract base class for any iterable object. Any class that implements the
core [iterator protocol][] can extend this to get a number of helpful methods.

[iterator protocol]: ../control-flow.html#the-iterator-protocol

## Methods

### **all**(predicate)

Tests whether all the elements in the sequence pass the `predicate`.

Iterates over the sequence, passing each element to the function `predicate`.
If it returns something [false](../control-flow.html#truth), stops iterating
and returns the value. Otherwise, returns `true`.

    :::dart
    [1, 2, 3].all {|n| n > 2} // False.
    [1, 2, 3].all {|n| n < 4} // True.

### **any**(predicate)

Tests whether any element in the sequence passes the `predicate`.

Iterates over the sequence, passing each element to the function `predicate`.
If it returns something [true](../control-flow.html#truth), stops iterating and
returns that value. Otherwise, returns `false`.

    :::dart
    [1, 2, 3].any {|n| n < 1} // False.
    [1, 2, 3].any {|n| n > 2} // True.

### **contains**(element)

Returns whether the sequence contains any element equal to the given element.

### **count**

The number of elements in the sequence.

Unless a more efficient override is available, this will iterate over the
sequence in order to determine how many elements it contains.

### **count**(predicate)

Returns the number of elements in the sequence that pass the `predicate`.

Iterates over the sequence, passing each element to the function `predicate`
and counting the number of times the returned value evaluates to `true`.

    :::dart
    [1, 2, 3].count {|n| n > 2} // 1.
    [1, 2, 3].count {|n| n < 4} // 3.

### **each**(function)

Iterates over the sequence, passing each element to the given `function`.

    :::dart
    ["one", "two", "three"].each {|word| IO.print(word) }

### **isEmpty**

Returns whether the sequence contains any elements.

This can be more efficient that `count == 0` because this does not iterate over
the entire sequence.

### **join**(sep)

Returns a string representation of the sequence. The string representations of
the elements in the sequence is concatenated with intervening occurrences of
`sep`.

It is a runtime error if `sep` is not a string.

### **join**

Calls `join` with the empty string as the separator.

### **map**(transformation)

Creates a new sequence that applies the `transformation` to each element in the
original sequence while it is iterated.

    :::dart
    var doubles = [1, 2, 3].map {|n| n * 2 }
    for (n in doubles) {
      IO.print(n) // "2", "4", "6".
    }

The returned sequence is *lazy*. It only applies the mapping when you iterate
over the sequence, and it does so by holding a reference to the original
sequence.

This means you can use `map(_)` for things like infinite sequences or sequences
that have side effects when you iterate over them. But it also means that
changes to the original sequence will be reflected in the mapped sequence.

To force eager evaluation, just call `.toList` on the result.

    :::dart
    var numbers = [1, 2, 3]
    var doubles = numbers.map {|n| n * 2 }.toList
    numbers.add(4)
    IO.print(doubles) // [2, 4, 6].

### **reduce**(function)

Reduces the sequence down to a single value. `function` is a function that takes
two arguments, the accumulator and sequence item and returns the new accumulator
value. The accumulator is initialized from the first item in the sequence. Then,
the function is invoked on each remaining item in the sequence, iteratively
updating the accumulator.

It is a runtime error to call this on an empty sequence.

### **reduce**(seed, function)

Similar to above, but uses `seed` for the initial value of the accumulator. If
the sequence is empty, returns `seed`.

### **toList**

Creates a [list](list.html) containing all the elements in the sequence.

    :::dart
    (1..3).toList  // [1, 2, 3].

If the sequence is already a list, this creates a copy of it.

### **where**(predicate)

Creates a new sequence containing only the elements from the original sequence
that pass the `predicate`.

During iteration, each element in the original sequence is passed to the
function `predicate`. If it returns `false`, the element is skipped.

    :::dart
    var odds = (1..10).where {|n| n % 2 == 1 }
    for (n in odds) {
      IO.print(n) // "1", "3", "5", "7", "9".
    }

The returned sequence is *lazy*. It only applies the filtering when you iterate
over the sequence, and it does so by holding a reference to the original
sequence.

This means you can use `where(_)` for things like infinite sequences or
sequences that have side effects when you iterate over them. But it also means
that changes to the original sequence will be reflected in the filtered
sequence.

To force eager evaluation, just call `.toList` on the result.

    :::dart
    var numbers = [1, 2, 3, 4, 5, 6]
    var odds = numbers.where {|n| n % 2 == 1 }.toList
    numbers.add(7)
    IO.print(odds) // [1, 3, 5].
