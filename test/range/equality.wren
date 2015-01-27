var a = 2..5
var b = 2..6
var c = 2...5
var d = 2...6

IO.print(a == 2..5) // expect: true
IO.print(a == 2..6) // expect: false
IO.print(c == 2...5) // expect: true
IO.print(c == 2...6) // expect: false

IO.print(b != 2..5) // expect: true
IO.print(b != 2..6) // expect: false
IO.print(d != 2...5) // expect: true
IO.print(d != 2...6) // expect: false

IO.print(a != c) // expect: true
IO.print(b != d) // expect: true