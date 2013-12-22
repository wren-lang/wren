var f = null

{
  var local = "local"
  f = fn {
    IO.write(local)
  }
}

f.call // expect: local
