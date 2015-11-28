fn f() {
  while (true) {
    var i = "i"
    return fn () { System.print(i) }
  }
}

var g = f()
g()
// expect: i
