var a = "a"
var b = "b"
var c = "c"

// Assignment is right-associative.
a = b = c
IO.write(a) // expect: c
IO.write(b) // expect: c
IO.write(c) // expect: c
