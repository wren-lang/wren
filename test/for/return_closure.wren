var f = fn {
  for (i in [1, 2, 3]) {
    return fn IO.print(i)
  }
}

var g = f.call
g.call
// expect: 1
