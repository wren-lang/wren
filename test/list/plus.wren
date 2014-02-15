var a = [1, 2, 3]
var b = [4, 5, 6]
var c = a + b
var d = a + (4..6)
var e = a + []
var f = [] + a
var g = [] + []

IO.print(a) // expect: [1, 2, 3]
IO.print(b) // expect: [4, 5, 6]
IO.print(c) // expect: [1, 2, 3, 4, 5, 6]
IO.print(d) // expect: [1, 2, 3, 4, 5, 6]
IO.print(e) // expect: [1, 2, 3]
IO.print(f) // expect: [1, 2, 3]
IO.print(g) // expect: []

// Doesn't modify original list.
IO.print(a) // expect: [1, 2, 3]
