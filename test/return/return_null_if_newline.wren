var f = new Fn {
  return
  IO.print("bad")
}

IO.print(f.call) // expect: null
