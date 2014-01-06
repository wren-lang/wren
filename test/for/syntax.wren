// Single-expression body.
for (i in [1, 2, 3]) IO.print(i)
// expect: 1
// expect: 2
// expect: 3

// Block body.
for (i in [1, 2, 3]) {
  IO.print(i)
}
// expect: 1
// expect: 2
// expect: 3

// Newline after "for".
for
(i in [1, 2, 3]) IO.print(i)
// expect: 1
// expect: 2
// expect: 3

// TODO: Return from inside for loop.
// TODO: Return closure from inside for loop.
