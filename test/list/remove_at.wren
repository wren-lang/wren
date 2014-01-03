var a = [1, 2, 3]
a.removeAt(0)
IO.write(a) // expect: [2, 3]

var b = [1, 2, 3]
b.removeAt(1)
IO.write(b) // expect: [1, 3]

var c = [1, 2, 3]
c.removeAt(2)
IO.write(c) // expect: [1, 2]

// Index backwards from end.
var d = [1, 2, 3]
d.removeAt(-3)
IO.write(d) // expect: [2, 3]

var e = [1, 2, 3]
e.removeAt(-2)
IO.write(e) // expect: [1, 3]

var f = [1, 2, 3]
f.removeAt(-1)
IO.write(f) // expect: [1, 2]

// Return the removed value.
IO.write([3, 4, 5].removeAt(1)) // expect: 4
