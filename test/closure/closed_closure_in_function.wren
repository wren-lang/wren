var f = null

{
  var local = "local"
  f = new Fn {
    IO.print(local)
  }
}

f.call // expect: local
