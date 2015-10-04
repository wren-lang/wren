^title Range Class
^category core

A range defines a bounded range of values from a starting point to a possibly
exclusive endpoint. [Here](../range.html) is a friendly introduction.

Extends [Sequence](sequence.html).

## Methods

### **from**

The starting point of the range. A range may be backwards, so this can be
greater than [to].

    :::wren
    (3..5).min // 3.
    (4..2).min // 4.

### **to**

The endpoint of the range. If the range is inclusive, this value is included,
otherwise it is not.

    :::wren
    (3..5).min // 5.
    (4..2).min // 2.

### **min**

The minimum bound of the range. Returns either `from`, or `to`, whichever is
lower.

    :::wren
    (3..5).min // 3.
    (4..2).min // 2.

### **max**

The maximum bound of the range. Returns either `from`, or `to`, whichever is
greater.

    :::wren
    (3..5).min // 5.
    (4..2).min // 4.

### **isInclusive**

Whether or not the range includes `to`. (`from` is always included.)

    :::wren
    (3..5).isInclusive   // true.
    (3...5).isInclusive  // false.

### **iterate**(iterator), **iteratorValue**(iterator)

Iterates over the range. Starts at `from` and increments by one towards `to`
until the endpoint is reached.
