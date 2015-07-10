var f = Fn.new {
  while (true) {
    var i = "i"
    return Fn.new { IO.print(i) }
  }
}

var g = f.call()
g.call()
// expect: i
