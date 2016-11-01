// Regression test.
//
// If you invoked some code with wrenCall() and that code aborted the current
// fiber, it did not reset the API stack. If you tried to immediately reuse the
// API stack by calling wrenCall(), it would be in a broken state.
class Test {
  static abortFiber() {
    Fiber.abort("Abort!") // expect handled runtime error: Abort!
  }

  static afterAbort(a, b) {
    System.print(a + b) // expect: 3
  }
}
