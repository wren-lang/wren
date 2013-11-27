var a = [1, 2, 3]
a.removeAt(0)
io.write(a) // expect: [2, 3]

var b = [1, 2, 3]
b.removeAt(1)
io.write(b) // expect: [1, 3]

var c = [1, 2, 3]
c.removeAt(2)
io.write(c) // expect: [1, 2]

// Index backwards from end.
var d = [1, 2, 3]
d.removeAt(-3)
io.write(d) // expect: [2, 3]

var e = [1, 2, 3]
e.removeAt(-2)
io.write(e) // expect: [1, 3]

var f = [1, 2, 3]
f.removeAt(-1)
io.write(f) // expect: [1, 2]

// Out of bounds.
// TODO(bob): Signal error in better way.
io.write([1, 2, 3].removeAt(3)) // expect: null
io.write([1, 2, 3].removeAt(-4)) // expect: null

// Return the removed value.
io.write([3, 4, 5].removeAt(1)) // expect: 4
