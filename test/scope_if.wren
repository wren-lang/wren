// Create a local scope for the 'then' expression.
var a = "out"
if (true) var a = "in"
io.write(a) // expect: out

// Create a local scope for the 'else' expression.
var b = "out"
if (false) "dummy" else var b = "in"
io.write(b) // expect: out
