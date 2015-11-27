fn f() {
  return
  System.print("bad")
}

System.print(f()) // expect: null
