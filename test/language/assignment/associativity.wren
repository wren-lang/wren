var a = "a"
var b = "b"
var c = "c"

// Assignment is right-associative.
a = b = c
System.print(a) // expect: c
System.print(b) // expect: c
System.print(c) // expect: c
