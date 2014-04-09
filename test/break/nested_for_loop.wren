for (i in 0..2) {
  IO.print("outer ", i)
  if (i > 1) break

  for (j in 0..2) {
    IO.print("inner ", j)
    if (j > 1) break
  }
}

// expect: outer 0
// expect: inner 0
// expect: inner 1
// expect: inner 2
// expect: outer 1
// expect: inner 0
// expect: inner 1
// expect: inner 2
// expect: outer 2
