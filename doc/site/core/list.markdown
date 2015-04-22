^title List Class
^category core

Extends [Sequence](sequence.html).

An indexable contiguous collection of elements. More details [here](../lists.html).

## Methods

### **add**(item)

Appends `item` to the end of the list.

### **clear**()

Removes all elements from the list.

### **count**

The number of elements in the list.

### **insert**(index, item)

Inserts the `item` at `index` in the list.

    :::dart
    var list = ["a", "b", "c", "d"]
    list.insert(1, "e")
    IO.print(list) // "[a, e, b, c, d]"

The `index` may be one past the last index in the list to append an element.

    :::dart
    var list = ["a", "b", "c"]
    list.insert(3, "d")
    IO.print(list) // "[a, b, c, d]"

If `index` is negative, it counts backwards from the end of the list. It bases this on the length of the list *after* inserted the element, so that `-1` will append the element, not insert it before the last element.

    :::dart
    var list = ["a", "b"]
    list.insert(-1, "d")
    list.insert(-2, "c")
    IO.print(list) // "[a, b, c, d]"

Returns the inserted item.

    :::dart
    IO.print(["a", "c"].insert(1, "b")) // "b".

It is a runtime error if the index is not an integer or is out of bounds.

### **iterate**(iterator), **iteratorValue**(iterator)

Implements the [iterator protocol](../control-flow.html#the-iterator-protocol)
for iterating over the elements in the list.

### **removeAt**(index)

Removes the element at `index`. If `index` is negative, it counts backwards
from the end of the list where `-1` is the last element. All trailing elements
are shifted up to fill in where the removed element was.

    :::dart
    var list = ["a", "b", "c", "d"]
    list.removeAt(1)
    IO.print(list) // "[a, c, d]".

Returns the removed item.

    IO.print(["a", "b", "c"].removeAt(1)) // "b".

It is a runtime error if the index is not an integer or is out of bounds.

### **[**index**]** operator

Gets the element at `index`. If `index` is negative, it counts backwards from
the end of the list where `-1` is the last element.

    :::dart
    var list = ["a", "b", "c"]
    IO.print(list[1]) // "b".

It is a runtime error if the index is not an integer or is out of bounds.

### **[**index**]=**(item) operator

Replaces the element at `index` with `item`. If `index` is negative, it counts
backwards from the end of the list where `-1` is the last element.

    :::dart
    var list = ["a", "b", "c"]
    list[1] = "new"
    IO.print(list) // "[a, new, c]".

It is a runtime error if the index is not an integer or is out of bounds.
