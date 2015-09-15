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

f.call()
// expect: local
// expect: after f

g.call()
// expect: after f
// expect: after g
