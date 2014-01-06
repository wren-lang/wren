var f = fn {
  while (true) {
    var i = "i"
    return fn IO.print(i)
  }
}

var g = f.call
g.call
// expect: i
