var a = [1, 2, 3]
var moreThan1 = fn (x) { return x > 1 }
var moreThan10 = fn (x) { return x > 10 }
var b = a.where(moreThan1)
var c = a.where(moreThan10)

IO.print(b) // expect: [2, 3]
IO.print(c) // expect: []
