// Evaluate the 'then' expression if the condition is true.
if (true) io.write("good") // expect: good
if (false) io.write("bad")

// Evaluate the 'else' expression if the condition is false.
if (true) io.write("good") else io.write("bad") // expect: good
if (false) io.write("bad") else io.write("good") // expect: good

// Allow blocks for branches.
if (true) { io.write("block") } // expect: block
if (false) null else { io.write("block") } // expect: block

// Assignment in if condition.
var a = false
if (a = true) io.write(a) // expect: true

// Newline after "if".
if
(true) io.write("good") // expect: good

// Newline after "else".
if (false) io.write("bad") else
io.write("good") // expect: good

// Only false is falsy.
if (0) io.write(0) // expect: 0
if (null) io.write(null) // expect: null
if ("") io.write("empty") // expect: empty
