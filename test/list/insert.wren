// Add to empty list.
var a = []
a.insert(1, 0)
IO.print(a) // expect: [1]

// Normal indices.
var b = [1, 2, 3]
b.insert(4, 0)
IO.print(b) // expect: [4, 1, 2, 3]

var c = [1, 2, 3]
c.insert(4, 1)
IO.print(c) // expect: [1, 4, 2, 3]

var d = [1, 2, 3]
d.insert(4, 2)
IO.print(d) // expect: [1, 2, 4, 3]

var e = [1, 2, 3]
e.insert(4, 3)
IO.print(e) // expect: [1, 2, 3, 4]

// Negative indices.
var f = [1, 2, 3]
f.insert(4, -4)
IO.print(f) // expect: [4, 1, 2, 3]

var g = [1, 2, 3]
g.insert(4, -3)
IO.print(g) // expect: [1, 4, 2, 3]

var h = [1, 2, 3]
h.insert(4, -2)
IO.print(h) // expect: [1, 2, 4, 3]

var i = [1, 2, 3]
i.insert(4, -1)
IO.print(i) // expect: [1, 2, 3, 4]

// Returns.inserted value.
IO.print([1, 2].insert(3, 0)) // expect: 3
