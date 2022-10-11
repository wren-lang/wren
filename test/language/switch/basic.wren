switch(1) {
  1: System.print("one")      // expect: one
  else: System.print("nope")
}

switch(2) {
  1: System.print("one")
  else: System.print("nope")  // expect: nope
}

switch ("qwe") {
  "qwe": {
    var n = 1
    System.print(n)  // expect: 1
  }
  else: System.print("nope")
}
