fn f() {
  while (true) {
    var i = "i"
    return Fn.new { System.print(i) }
  }
}

var g = f()
g()
// expect: i
