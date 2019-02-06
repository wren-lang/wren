^title Lists

A list is a compound object that holds a collection of elements identified by
integer index. You can create a list by placing a sequence of comma-separated
expressions inside square brackets:

    :::wren
    [1, "banana", true]

Here, we've created a list of three elements. Notice that the elements don't
have to be the same type.

## Accessing elements

You can access an element from a list by calling the [subscript
operator][] on it with the index of the
element you want. Like most languages, indexes start at zero:

[subscript operator]: method-calls.html#subscripts

    :::wren
    var hirsute = ["sideburns", "porkchops", "'stache", "goatee"]
    System.print(hirsute[0]) //> sideburns
    System.print(hirsute[1]) //> porkchops

Negative indices counts backwards from the end:

    :::wren
    System.print(hirsute[-1]) //> goatee
    System.print(hirsute[-2]) //> 'stache

It's a runtime error to pass an index outside of the bounds of the list. If you
don't know what those bounds are, you can find out using count:

    :::wren
    System.print(hirsute.count) //> 4

## Slices and ranges

Sometimes you want to copy a chunk of elements from a list. You can do that by
passing a [range](values.html#ranges) to the subscript operator, like so:

    :::wren
    System.print(hirsute[1..2]) //> [porkchops, 'stache]

This returns a new list containing the elements of the original list whose
indices are within the given range. Both inclusive and exclusive ranges work
and do what you expect.

Negative bounds also work like they do when passing a single number, so to copy
a list, you can just do:

    :::wren
    hirsute[0..-1]

## Adding elements

Lists are *mutable*, meaning their contents can be changed. You can swap out an
existing element in the list using the subscript setter:

    :::wren
    hirsute[1] = "muttonchops"
    System.print(hirsute[1]) //> muttonchops

It's an error to set an element that's out of bounds. To grow a list, you can
use `add` to append a single item to the end:

    :::wren
    hirsute.add("goatee")
    System.print(hirsute.count) //> 4

You can insert a new element at a specific position using `insert`:

    :::wren
    hirsute.insert(2, "soul patch")

The first argument is the index to insert at, and the second is the value to
insert. All elements following the inserted one will be pushed down to
make room for it.

It's valid to "insert" after the last element in the list, but only *right*
after it. Like other methods, you can use a negative index to count from the
back. Doing so counts back from the size of the list *after* it's grown by one:

    :::wren
    var letters = ["a", "b", "c"]
    letters.insert(3, "d")   // OK: inserts at end.
    System.print(letters)    //> [a, b, c, d]
    letters.insert(-2, "e")  // Counts back from size after insert.
    System.print(letters)    //> [a, b, c, e, d]

## Adding lists together

Lists have the ability to be added together via the `+` operator. This is often known as concatenation.

    :::wren
    var letters = ["a", "b", "c"]
    var other = ["d", "e", "f"]
    var combined = letters + other
    System.print(combined)  //> [a, b, c, d, e, f]

## Removing elements

The opposite of `insert` is `removeAt`. It removes a single element from a
given position in the list. All following items are shifted up to fill in the
gap:

    :::wren
    var letters = ["a", "b", "c", "d"]
    letters.removeAt(1)
    System.print(letters) //> [a, c, d]

The `removeAt` method returns the removed item:

    :::wren
    System.print(letters.removeAt(1)) //> c

If you want to remove everything from the list, you can clear it:

    :::wren
    hirsute.clear()
    System.print(hirsute) //> []

<br><hr>
<a class="right" href="maps.html">Maps &rarr;</a>
<a href="values.html">&larr; Values</a>
