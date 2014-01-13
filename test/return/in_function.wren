var f = fn {
  return "ok"
  IO.print("bad")
}

IO.print(f.call) // expect: ok
