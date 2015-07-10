var f = null

{
  var local = "local"
  f = Fn.new {
    IO.print(local)
  }
}

f.call() // expect: local
