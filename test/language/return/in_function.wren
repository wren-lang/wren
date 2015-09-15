var f = Fn.new {
  return "ok"
  System.print("bad")
}

System.print(f.call()) // expect: ok
