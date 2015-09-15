var a = 2..5
var b = 2..6
var c = 2...5
var d = 2...6

System.print(a == 2..5) // expect: true
System.print(a == 2..6) // expect: false
System.print(c == 2...5) // expect: true
System.print(c == 2...6) // expect: false

System.print(b != 2..5) // expect: true
System.print(b != 2..6) // expect: false
System.print(d != 2...5) // expect: true
System.print(d != 2...6) // expect: false

System.print(a != c) // expect: true
System.print(b != d) // expect: true