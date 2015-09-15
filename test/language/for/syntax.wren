// Single-expression body.
for (i in [1]) System.print(i)
// expect: 1

// Block body.
for (i in [1]) {
  System.print(i)
}
// expect: 1

// Newline after "in".
for (i in
  [1]) System.print(i)
// expect: 1