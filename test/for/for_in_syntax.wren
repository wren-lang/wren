// Single-expression body.
for (i in [1, 2, 3]) IO.write(i)
// expect: 1
// expect: 2
// expect: 3

// Block body.
for (i in [1, 2, 3]) {
  IO.write(i)
}
// expect: 1
// expect: 2
// expect: 3

// Newline after "for".
for
(i in [1, 2, 3]) IO.write(i)
// expect: 1
// expect: 2
// expect: 3
