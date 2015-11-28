var f = null
var g = null

{
  var local = "local"
  f = fn () {
    System.print(local)
    local = "after f"
    System.print(local)
  }

  g = fn () {
    System.print(local)
    local = "after g"
    System.print(local)
  }
}

f()
// expect: local
// expect: after f

g()
// expect: after f
// expect: after g
