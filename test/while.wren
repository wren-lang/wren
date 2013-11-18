// Single-expression body.
var c = 0
while (c < 3) io.write(c = c + 1)
// expect: 1
// expect: 2
// expect: 3

// Block body.
var a = 0
while (a < 3) {
    io.write(a)
    a = a + 1
}
// expect: 0
// expect: 1
// expect: 2

// Newline after "while".
var d = 0
while
(d < 3) io.write(d = d + 1)
// expect: 1
// expect: 2
// expect: 3

// Result is null.
var e = 0
var f = while (e < 3) {
  e = e + 1
}
io.write(f) // expect: null
