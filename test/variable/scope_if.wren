// Create a local scope for the 'then' expression.
var a = "out"
if (true) {
  var a = "in"
}
IO.write(a) // expect: out

// Create a local scope for the 'else' expression.
var b = "out"
if (false) "dummy" else {
  var b = "in"
}
IO.write(b) // expect: out
