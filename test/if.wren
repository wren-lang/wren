// Evaluate the 'then' expression if the condition is true.
if (true) IO.print("good") // expect: good
if (false) IO.print("bad")

// Evaluate the 'else' expression if the condition is false.
if (true) IO.print("good") else IO.print("bad") // expect: good
if (false) IO.print("bad") else IO.print("good") // expect: good

// Allow blocks for branches.
if (true) { IO.print("block") } // expect: block
if (false) null else { IO.print("block") } // expect: block

// Assignment in if condition.
var a = false
if (a = true) IO.print(a) // expect: true

// Newline after "if".
if
(true) IO.print("good") // expect: good

// Newline after "else".
if (false) IO.print("bad") else
IO.print("good") // expect: good

// Only false is falsy.
if (0) IO.print(0) // expect: 0
if (null) IO.print(null) // expect: null
if ("") IO.print("empty") // expect: empty
