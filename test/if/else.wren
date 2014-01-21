// Evaluate the 'else' expression if the condition is false.
if (true) IO.print("good") else IO.print("bad") // expect: good
if (false) IO.print("bad") else IO.print("good") // expect: good

// Allow block body.
if (false) null else { IO.print("block") } // expect: block

// Newline after "else".
if (false) IO.print("bad") else
IO.print("good") // expect: good
