// Evaluate the 'else' expression if the condition is false.
if (true) System.print("good") else System.print("bad") // expect: good
if (false) System.print("bad") else System.print("good") // expect: good

// Allow block body.
if (false) null else { System.print("block") } // expect: block
