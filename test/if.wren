// Evaluate the 'then' expression if the condition is true.
if (true) IO.write("good") // expect: good
if (false) IO.write("bad")

// Evaluate the 'else' expression if the condition is false.
if (true) IO.write("good") else IO.write("bad") // expect: good
if (false) IO.write("bad") else IO.write("good") // expect: good

// Allow blocks for branches.
if (true) { IO.write("block") } // expect: block
if (false) null else { IO.write("block") } // expect: block

// Assignment in if condition.
var a = false
if (a = true) IO.write(a) // expect: true

// Newline after "if".
if
(true) IO.write("good") // expect: good

// Newline after "else".
if (false) IO.write("bad") else
IO.write("good") // expect: good

// Only false is falsy.
if (0) IO.write(0) // expect: 0
if (null) IO.write(null) // expect: null
if ("") IO.write("empty") // expect: empty
