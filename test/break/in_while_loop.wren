var i = 0
while (true) {
  i = i + 1
  IO.write(i)
  if (i > 2) {
    // TODO: Should not require block for break.
    break
  }
  IO.write(i)
}
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
