var a = [1]
a.addAll([2, 3])
IO.print(a) // expect: [1, 2, 3]
a.addAll([])
IO.print(a) // expect: [1, 2, 3]
a.addAll(4..6)
IO.print(a) // expect: [1, 2, 3, 4, 5, 6]

// Returns argument.
var range = 7..9
IO.print(a.addAll(range) == range) // expect: true
