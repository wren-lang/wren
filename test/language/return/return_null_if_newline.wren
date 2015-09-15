var f = Fn.new {
  return
  System.print("bad")
}

System.print(f.call()) // expect: null
