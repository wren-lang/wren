class Foo {
  construct new() {
    return 1 // expect error: Cannot return a value from constructor.
  }
}
