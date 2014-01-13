var f = fn {
  if (true) { return }
  IO.print("bad")
}

IO.print(f.call) // expect: null
