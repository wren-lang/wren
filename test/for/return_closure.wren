var f = new Fn {
  for (i in [1, 2, 3]) {
    return new Fn { IO.print(i) }
  }
}

var g = f.call
g.call
// expect: 1
