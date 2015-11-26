var f = null
var g = null

{
  var local = "local"
  f = Fn.new {
    System.print(local)
    local = "after f"
    System.print(local)
  }

  g = Fn.new {
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
