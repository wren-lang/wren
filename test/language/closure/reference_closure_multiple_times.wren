var f = null

{
  var a = "a"
  f = Fn.new {
    System.print(a)
    System.print(a)
  }
}

f.call()
// expect: a
// expect: a
