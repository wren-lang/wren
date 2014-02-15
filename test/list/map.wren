var a = [1, 2, 3]
var inc = fn (x) { return x + 1 }
var b = a.map(inc)

IO.print(b) // expect: [2, 3, 4]
