// Evaluate the 'then' expression if the condition is true.
if (true) io.write("good") // expect: good
if (false) io.write("bad")

// Evaluate the 'else' expression if the condition is false.
if (true) io.write("good") else io.write("bad") // expect: good
if (false) io.write("bad") else io.write("good") // expect: good

// Allow statements for then branch.
if (true) { io.write("block") } // expect: block
if (true) if (true) io.write("double") // expect: double

// Allow statements for else branch.
if (false) null else { io.write("block") } // expect: block
if (false) null else if (true) io.write("double") // expect: double

// Return the 'then' expression if the condition is true.
var a = if (true) "good"
io.write(a) // expect: good

// Return null if the condition is false and there is no else.
var b = if (false) "bad"
io.write(b) // expect: null

// Return the 'else' expression if the condition is false.
var c = if (false) "bad" else "good"
io.write(c) // expect: good

// Assignment in if condition.
if (a = true) io.write(a) // expect: true

// Newline after "if".
if
(true) io.write("good") // expect: good

// Newline after "else".
if (false) io.write("bad") else
io.write("good") // expect: good
