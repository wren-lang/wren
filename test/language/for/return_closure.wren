fn f() {
  for (i in [1, 2, 3]) {
    return fn () { System.print(i) }
  }
}

var g = f()
g()
// expect: 1
