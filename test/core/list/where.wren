var a = [1, 2, 3]
var b = a.where {|x| x > 1 }.toList
System.print(b) // expect: [2, 3]

var c = a.where {|x| x > 10 }.toList
System.print(c) // expect: []
