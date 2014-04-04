// Single-expression body.
for (i in [1]) IO.print(i)
// expect: 1

// Block body.
for (i in [1]) {
  IO.print(i)
}
// expect: 1

// Newline after "in".
for (i in
  [1]) IO.print(i)
// expect: 1