// This is a regression test. There was a bug where if an upvalue for an
// earlier local (here "a") was captured *after* a later one ("b"), then Wren
// would crash because it walked to the end of the upvalue list (correct), but
// then didn't handle not finding the variable.

fn () {
  var a = "a"
  var b = "b"
  fn () {
    System.print(b) // expect: b
    System.print(a) // expect: a
  }()
}()
