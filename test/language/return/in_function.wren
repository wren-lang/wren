fn f() {
  return "ok"
  System.print("bad")
}

System.print(f()) // expect: ok
