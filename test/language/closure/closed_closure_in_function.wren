var f = null

{
  var local = "local"
  f = fn () {
    System.print(local)
  }
}

f() // expect: local
