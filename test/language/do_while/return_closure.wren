var f = Fn.new {
  do {
    var i = "i"
    return Fn.new { System.print(i) }
  } while(true)
}

var g = f.call()
g.call()
// expect: i
