fn block(arg) { arg }
var f = block {
  // Hi.
}
System.print(f()) // expect: null
