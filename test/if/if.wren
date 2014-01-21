// Evaluate the 'then' expression if the condition is true.
if (true) IO.print("good") // expect: good
if (false) IO.print("bad")

// Allow block body.
if (true) { IO.print("block") } // expect: block

// Assignment in if condition.
var a = false
if (a = true) IO.print(a) // expect: true

// Newline after "if".
if
(true) IO.print("good") // expect: good
