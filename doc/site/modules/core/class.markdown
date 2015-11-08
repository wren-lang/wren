^title Class Class

**TODO**

## Methods

### **name**

The name of the class.

### **supertype**

The superclass of this class.

    :::wren
    class Crustacean {}
    class Crab is Crustacean {}

    System.print(Crab.supertype) //> Crustacean

A class with no explicit superclass implicitly inherits Object:

    :::wren
    System.print(Crustacean.supertype) //> Object

Object forms the root of the class hierarchy and has no supertype:

    :::wren
    System.print(Object.supertype) //> null
