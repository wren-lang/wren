// Evaluate the 'then' expression if the condition is true.
if (true) System.print("good") // expect: good
if (false) System.print("bad")

// Allow block body.
if (true) { System.print("block") } // expect: block

// Assignment in if condition.
var a = false
if (a = true) System.print(a) // expect: true
