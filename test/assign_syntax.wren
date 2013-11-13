// Chained assignment.
var a = "a"
var b = "b"
a = b = "chain"
io.write(a) // expect: chain
io.write(a) // expect: chain

// Assignment on RHS of variable.
var c = a = "var"
io.write(a) // expect: var
io.write(c) // expect: var
