// TODO: What happens if the index isn't an integer?
// TODO: Out of bounds.

// Add to empty list.
var a = []
a.insert(1, 0)
io.write(a) // expect: [1]

// Normal indices.
var b = [1, 2, 3]
b.insert(4, 0)
io.write(b) // expect: [4, 1, 2, 3]

var c = [1, 2, 3]
c.insert(4, 1)
io.write(c) // expect: [1, 4, 2, 3]

var d = [1, 2, 3]
d.insert(4, 2)
io.write(d) // expect: [1, 2, 4, 3]

var e = [1, 2, 3]
e.insert(4, 3)
io.write(e) // expect: [1, 2, 3, 4]

// Negative indices.
var f = [1, 2, 3]
f.insert(4, -4)
io.write(f) // expect: [4, 1, 2, 3]

var g = [1, 2, 3]
g.insert(4, -3)
io.write(g) // expect: [1, 4, 2, 3]

var h = [1, 2, 3]
h.insert(4, -2)
io.write(h) // expect: [1, 2, 4, 3]

var i = [1, 2, 3]
i.insert(4, -1)
io.write(i) // expect: [1, 2, 3, 4]

// Returns.inserted value.
io.write([1, 2].insert(3, 0)) // expect: 3
