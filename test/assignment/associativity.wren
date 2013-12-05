var a = "a"
var b = "b"
var c = "c"

// Assignment is right-associative.
a = b = c
io.write(a) // expect: c
io.write(b) // expect: c
io.write(c) // expect: c
