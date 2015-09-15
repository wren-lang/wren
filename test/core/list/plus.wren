var a = [1, 2, 3]
var b = [4, 5, 6]
var c = a + b
var d = a + (4..6)
var e = a + []
var f = [] + a
var g = [] + []

System.print(a) // expect: [1, 2, 3]
System.print(b) // expect: [4, 5, 6]
System.print(c) // expect: [1, 2, 3, 4, 5, 6]
System.print(d) // expect: [1, 2, 3, 4, 5, 6]
System.print(e) // expect: [1, 2, 3]
System.print(f) // expect: [1, 2, 3]
System.print(g) // expect: []

// Doesn't modify original list.
System.print(a) // expect: [1, 2, 3]
