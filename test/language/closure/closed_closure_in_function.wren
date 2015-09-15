var f = null

{
  var local = "local"
  f = Fn.new {
    System.print(local)
  }
}

f.call() // expect: local
