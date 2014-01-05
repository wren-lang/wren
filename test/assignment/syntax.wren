// Chained assignment.
var a = "a"
var b = "b"
a = b = "chain"
IO.print(a) // expect: chain
IO.print(a) // expect: chain

// Assignment on RHS of variable.
var c = a = "var"
IO.print(a) // expect: var
IO.print(c) // expect: var
