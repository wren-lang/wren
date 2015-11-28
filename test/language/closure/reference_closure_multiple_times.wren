var f = null

{
  var a = "a"
  f = fn () {
    System.print(a)
    System.print(a)
  }
}

f()
// expect: a
// expect: a
