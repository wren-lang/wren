// Evaluate the 'then' expression if the condition is true.
if (true) io.write("good") // expect: good
if (false) io.write("bad")

// Evaluate the 'else' expression if the condition is false.
if (true) io.write("good") else io.write("bad") // expect: good
if (false) io.write("bad") else io.write("good") // expect: good

// Return the 'then' expression if the condition is true.
io.write(if (true) "good") // expect: good

// Return null if the condition is false and there is no else.
io.write(if (false) "bad") // expect: null

// Return the 'else' expression if the condition is false.
io.write(if (false) "bad" else "good") // expect: good
