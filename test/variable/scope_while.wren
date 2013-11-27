// Body has its own scope.
var a = "outer"
var i = 0
while ((i = i + 1) <= 1) var a = "inner"
io.write(a) // expect: outer

// TODO(bob): What about condition?
