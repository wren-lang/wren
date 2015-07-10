var f = Fn.new {
  for (i in [1, 2, 3]) {
    return Fn.new { IO.print(i) }
  }
}

var g = f.call()
g.call()
// expect: 1
