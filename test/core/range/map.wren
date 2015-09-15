var a = 1..3
var b = a.map {|x| x + 1 }.toList
System.print(b) // expect: [2, 3, 4]
