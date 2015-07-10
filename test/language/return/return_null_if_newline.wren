var f = Fn.new {
  return
  IO.print("bad")
}

IO.print(f.call()) // expect: null
