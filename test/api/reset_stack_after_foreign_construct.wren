// Regression test.
//
// After a foreign constructor was called, it did not reset the API stack. If
// you tried to immediately reuse the API stack by calling wrenCall(), it
// would be in a broken state.
foreign class ResetStackForeign {
  construct new(a) {}
}

class Test {
  static callConstruct() {
    ResetStackForeign.new(1)
  }

  static afterConstruct(a, b) {
    System.print(a + b) // expect: 3
  }
}
