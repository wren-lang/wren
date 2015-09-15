^title Class Class
^category core

## Methods

### **name**

The name of the class.

### **supertype**

The superclass of this class.

    :::dart
    class Crustacean {}
    class Crab is Crustacean {}

    System.print(Crab.supertype) // "Crustacean".

A class with no explicit superclass implicitly inherits Object:

    :::dart
    System.print(Crustacean.supertype) // "Object".

Object forms the root of the class hierarchy and has no supertype:

    :::dart
    System.print(Object.supertype) // "null".
