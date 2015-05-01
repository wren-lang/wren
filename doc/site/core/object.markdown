^title Object Class
^category core

## Static Methods

## **same**(obj1, obj2)

Returns `true` if *obj1* and *obj2* are the same. For [value
types](../values.html), this returns `true` if the objects have equivalent
state. In other words, numbers, strings, booleans, and ranges compare by value.

For all other objects, this returns `true` only if *obj1* and *obj2* refer to
the exact same object in memory.

This is similar to the built in `==` operator in Object except that this cannot
be overriden. It allows you to reliably access the built-in equality semantics
even on user-defined classes.

## Methods

### **!** operator

Returns `false`, since most objects are considered [true](control-flow.html#truth).

### **==**(other) and **!=**(other) operators

Compares two objects using built-in equality. This compares [value
types](../values.html) by value, and all other objects are compared by
identity&mdash;two objects are equal only if they are the exact same object.

### **toString**

A default string representation of the object.

### **type**

The [Class](#class-class) of the object.
