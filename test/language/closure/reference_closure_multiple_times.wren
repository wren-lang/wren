var f = null

{
  var a = "a"
  f = Fn.new {
    IO.print(a)
    IO.print(a)
  }
}

f.call()
// expect: a
// expect: a
