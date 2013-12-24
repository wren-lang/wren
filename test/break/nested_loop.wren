var i = 0
while (true) {
  IO.write("outer " + i.toString)
  if (i > 1) {
    // TODO: Should not require block for break.
    break
  }

  var j = 0
  while (true) {
    IO.write("inner " + j.toString)
    if (j > 1) {
      // TODO: Should not require block for break.
      break
    }

    j = j + 1
  }

  i = i + 1
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
