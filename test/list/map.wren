var a = [1,2,3]
var inc = fn (x) { return x + 1 }
var b = a.map(inc)

IO.write(b) // expect: [2, 3, 4]
