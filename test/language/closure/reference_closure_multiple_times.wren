var f = null

{
  var a = "a"
  f = Fn.new {
    System.print(a)
    System.print(a)
  }
}

f()
// expect: a
// expect: a
