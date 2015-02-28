var f = new Fn {
  return "ok"
  IO.print("bad")
}

IO.print(f.call()) // expect: ok
