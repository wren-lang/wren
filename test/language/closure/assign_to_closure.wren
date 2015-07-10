var f = null
var g = null

{
  var local = "local"
  f = Fn.new {
    IO.print(local)
    local = "after f"
    IO.print(local)
  }

  g = Fn.new {
    IO.print(local)
    local = "after g"
    IO.print(local)
  }
}

f.call()
// expect: local
// expect: after f

g.call()
// expect: after f
// expect: after g
