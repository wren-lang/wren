// Create a local scope for the 'then' expression.
var a = "out"
if (true) {
  var a = "in"
}
IO.print(a) // expect: out

// Create a local scope for the 'else' expression.
var b = "out"
if (false) "dummy" else {
  var b = "in"
}
IO.print(b) // expect: out
