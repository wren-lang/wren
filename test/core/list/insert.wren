// Add to empty list.
var a = []
a.insert(0, 1)
System.print(a) // expect: [1]

// Normal indices.
var b = [1, 2, 3]
b.insert(0, 4)
System.print(b) // expect: [4, 1, 2, 3]

var c = [1, 2, 3]
c.insert(1, 4)
System.print(c) // expect: [1, 4, 2, 3]

var d = [1, 2, 3]
d.insert(2, 4)
System.print(d) // expect: [1, 2, 4, 3]

var e = [1, 2, 3]
e.insert(3, 4)
System.print(e) // expect: [1, 2, 3, 4]

// Negative indices.
var f = [1, 2, 3]
f.insert(-4, 4)
System.print(f) // expect: [4, 1, 2, 3]

var g = [1, 2, 3]
g.insert(-3, 4)
System.print(g) // expect: [1, 4, 2, 3]

var h = [1, 2, 3]
h.insert(-2, 4)
System.print(h) // expect: [1, 2, 4, 3]

var i = [1, 2, 3]
i.insert(-1, 4)
System.print(i) // expect: [1, 2, 3, 4]

// Returns.inserted value.
System.print([1, 2].insert(0, 3)) // expect: 3
