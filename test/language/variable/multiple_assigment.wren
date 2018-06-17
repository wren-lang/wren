var a = [1, 2, 3]
System.print(a) // expect: [1, 2, 3]

var b, c = [1, "foo"]
System.print(b) // expect: 1
System.print(c) // expect: foo

var d, e = ["bar", 2, 3]
System.print(d) // expect: bar
System.print(e) // expect: 2

var f, g, h = [1, 2]
System.print(f) // expect: 1
System.print(g) // expect: 2
System.print(h) // expect: null

var i, j = 1 // expect assert failed: Must unpack a list.