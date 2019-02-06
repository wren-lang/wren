^title List Class

Extends [Sequence](sequence.html).

An indexable contiguous collection of elements. More details [here][lists].

[lists]: ../../lists.html

## Static Methods

### List.**filled**(size, element)

Creates a new list with `size` elements, all set to `element`.

It is a runtime error if `size` is not a nonnegative integer.

### List.**new**()

Creates a new empty list. Equivalent to `[]`.

## Methods

### **add**(item)

Appends `item` to the end of the list.

### **clear**()

Removes all elements from the list.

### **count**

The number of elements in the list.

### **insert**(index, item)

Inserts the `item` at `index` in the list.

    :::wren
    var list = ["a", "b", "c", "d"]
    list.insert(1, "e")
    System.print(list) //> [a, e, b, c, d]

The `index` may be one past the last index in the list to append an element.

    :::wren
    var list = ["a", "b", "c"]
    list.insert(3, "d")
    System.print(list) //> [a, b, c, d]

If `index` is negative, it counts backwards from the end of the list. It bases this on the length of the list *after* inserted the element, so that `-1` will append the element, not insert it before the last element.

    :::wren
    var list = ["a", "b"]
    list.insert(-1, "d")
    list.insert(-2, "c")
    System.print(list) //> [a, b, c, d]

Returns the inserted item.

    :::wren
    System.print(["a", "c"].insert(1, "b")) //> b

It is a runtime error if the index is not an integer or is out of bounds.

### **iterate**(iterator), **iteratorValue**(iterator)

Implements the [iterator protocol][] for iterating over the elements in the
list.

[iterator protocol]: ../../control-flow.html#the-iterator-protocol

### **removeAt**(index)

Removes the element at `index`. If `index` is negative, it counts backwards
from the end of the list where `-1` is the last element. All trailing elements
are shifted up to fill in where the removed element was.

    :::wren
    var list = ["a", "b", "c", "d"]
    list.removeAt(1)
    System.print(list) //> [a, c, d]

Returns the removed item.

    :::wren
    System.print(["a", "b", "c"].removeAt(1)) //> b

It is a runtime error if the index is not an integer or is out of bounds.

### **[**index**]** operator

Gets the element at `index`. If `index` is negative, it counts backwards from
the end of the list where `-1` is the last element.

    :::wren
    var list = ["a", "b", "c"]
    System.print(list[1]) //> b

It is a runtime error if the index is not an integer or is out of bounds.

### **[**index**]=**(item) operator

Replaces the element at `index` with `item`. If `index` is negative, it counts
backwards from the end of the list where `-1` is the last element.

    :::wren
    var list = ["a", "b", "c"]
    list[1] = "new"
    System.print(list) //> [a, new, c]

It is a runtime error if the index is not an integer or is out of bounds.

##  **+**(other) operator

 Appends a list to the end of the list (concatenation). `other` must be a `List`.

    :::wren
    var letters = ["a", "b", "c"]
    var other = ["d", "e", "f"]
    var combined = letters + other
    System.print(combined)  //> [a, b, c, d, e, f]
