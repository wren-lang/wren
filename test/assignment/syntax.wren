// Chained assignment.
var a = "a"
var b = "b"
a = b = "chain"
IO.write(a) // expect: chain
IO.write(a) // expect: chain

// Assignment on RHS of variable.
var c = a = "var"
IO.write(a) // expect: var
IO.write(c) // expect: var
