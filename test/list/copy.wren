var a = [1, 2, 3]
var b = a.copy
var c = []
var d = c.copy

IO.print(b) // expect: [1, 2, 3]
IO.print(a == b) // expect: false
IO.print(d) // expect: []
IO.print(c == d) // expect: false
