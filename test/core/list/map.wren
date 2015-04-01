var a = [1, 2, 3]
var b = a.map {|x| x + 1 }.list
IO.print(b) // expect: [2, 3, 4]
