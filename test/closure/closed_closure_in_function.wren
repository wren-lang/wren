var f = null

{
  var local = "local"
  f = fn {
    io.write(local)
  }
}

f.call // expect: local
