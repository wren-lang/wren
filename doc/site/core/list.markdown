^title List Class
^category core

Extends [Sequence](sequence.html).

An indexable contiguous collection of elements. More details [here](../lists.html).

### **add**(item)

Appends `item` to the end of the list.

### **clear**

Removes all items from the list.

### **count**

The number of items in the list.

### **insert**(item, index)

**TODO**

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
