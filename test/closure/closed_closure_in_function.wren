var f = null

{
  var local = "local"
  f = fn {
    IO.print(local)
  }
}

f.call // expect: local
