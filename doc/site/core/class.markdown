^title Class Class
^category core

### **name**

The name of the class.

### **supertype**

The superclass of this class.

    :::dart
    class Crustacean {}
    class Crab is Crustacean {}

    IO.print(Crab.supertype)        // "Crustacean"

A class with no explicit superclass implicitly inherits Object:

    :::dart
    IO.print(Crustacean.supertype)  // "Object"

Object forms the root of the class hierarchy and has no supertype:

    :::dart
    IO.print(Object.supertype)  // "null"
