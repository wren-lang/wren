// Chained assignment.
var a = "a"
var b = "b"
a = b = "chain"
System.print(a) // expect: chain
System.print(b) // expect: chain

// Assignment on RHS of variable.
var c = a = "var"
System.print(a) // expect: var
System.print(c) // expect: var
