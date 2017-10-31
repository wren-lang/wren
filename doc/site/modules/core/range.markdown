^title Range Class

A range defines a bounded range of values from a starting point to a possibly
exclusive endpoint. [Here](../../values.html#ranges) is a friendly introduction.

Extends [Sequence](sequence.html).

## Methods

### **from**

The starting point of the range. A range may be backwards, so this can be
greater than [to].

    :::wren
    System.print((3..5).from) //> 3
    System.print((4..2).from) //> 4

### **to**

The endpoint of the range. If the range is inclusive, this value is included,
otherwise it is not.

    :::wren
    System.print((3..5).to) //> 5
    System.print((4..2).to) //> 2

### **min**

The minimum bound of the range. Returns either `from`, or `to`, whichever is
lower.

    :::wren
    System.print((3..5).min) //> 3
    System.print((4..2).min) //> 2

### **max**

The maximum bound of the range. Returns either `from`, or `to`, whichever is
greater.

    :::wren
    System.print((3..5).max) //> 5
    System.print((4..2).max) //> 4

### **isInclusive**

Whether or not the range includes `to`. (`from` is always included.)

    :::wren
    System.print((3..5).isInclusive)   //> true
    System.print((3...5).isInclusive)  //> false

### **iterate**(iterator), **iteratorValue**(iterator)

Iterates over the range. Starts at `from` and increments by one towards `to`
until the endpoint is reached.
