var f = fn {
  return
  IO.print("bad")
}

IO.print(f.call) // expect: null
