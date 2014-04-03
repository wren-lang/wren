var f = null

{
  var a = "a"
  f = new Fn {
    IO.print(a)
    IO.print(a)
  }
}

f.call
// expect: a
// expect: a
