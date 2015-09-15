// Create a local scope for the 'then' expression.
var a = "out"
if (true) {
  var a = "in"
}
System.print(a) // expect: out

// Create a local scope for the 'else' expression.
var b = "out"
if (false) "dummy" else {
  var b = "in"
}
System.print(b) // expect: out
