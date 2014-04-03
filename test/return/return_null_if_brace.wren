var f = new Fn {
  if (true) { return }
  IO.print("bad")
}

IO.print(f.call) // expect: null
