var f = Fn.new {
  return "ok"
  IO.print("bad")
}

IO.print(f.call()) // expect: ok
