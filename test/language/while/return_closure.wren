var f = Fn.new {
  while (true) {
    var i = "i"
    return Fn.new { System.print(i) }
  }
}

var g = f.call()
g.call()
// expect: i
