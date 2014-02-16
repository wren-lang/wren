var a = 1..3
var b = a.map(fn (x) x + 1)
IO.print(b) // expect: [2, 3, 4]
