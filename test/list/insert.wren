// TODO: What happens if the index isn't an integer?
// TODO: Out of bounds.

// Add to empty list.
var a = []
a.insert(1, 0)
IO.write(a) // expect: [1]

// Normal indices.
var b = [1, 2, 3]
b.insert(4, 0)
IO.write(b) // expect: [4, 1, 2, 3]

var c = [1, 2, 3]
c.insert(4, 1)
IO.write(c) // expect: [1, 4, 2, 3]

var d = [1, 2, 3]
d.insert(4, 2)
IO.write(d) // expect: [1, 2, 4, 3]

var e = [1, 2, 3]
e.insert(4, 3)
IO.write(e) // expect: [1, 2, 3, 4]

// Negative indices.
var f = [1, 2, 3]
f.insert(4, -4)
IO.write(f) // expect: [4, 1, 2, 3]

var g = [1, 2, 3]
g.insert(4, -3)
IO.write(g) // expect: [1, 4, 2, 3]

var h = [1, 2, 3]
h.insert(4, -2)
IO.write(h) // expect: [1, 2, 4, 3]

var i = [1, 2, 3]
i.insert(4, -1)
IO.write(i) // expect: [1, 2, 3, 4]

// Returns.inserted value.
IO.write([1, 2].insert(3, 0)) // expect: 3
