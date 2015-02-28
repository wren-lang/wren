var f = new Fn {
  while (true) {
    var i = "i"
    return new Fn { IO.print(i) }
  }
}

var g = f.call()
g.call()
// expect: i
