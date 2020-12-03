for (i in 0..2) {
  System.print("outer %(i)")
  if (i == 1) continue

  for (j in 0..2) {
    if (j == 1) continue
    System.print("inner %(j)")
  }
}

// expect: outer 0
// expect: inner 0
// expect: inner 2
// expect: outer 1
// expect: outer 2
// expect: inner 0
// expect: inner 2