var f = null

{
  var a = "a"
  f = fn {
    IO.print(a)
    IO.print(a)
  }
}

f.call
// expect: a
// expect: a
