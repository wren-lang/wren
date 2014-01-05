var a = "a"
var b = "b"
var c = "c"

// Assignment is right-associative.
a = b = c
IO.print(a) // expect: c
IO.print(b) // expect: c
IO.print(c) // expect: c
