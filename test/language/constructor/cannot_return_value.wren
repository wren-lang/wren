class Foo {
  construct new() {
    return 1 // expect error: A constructor cannot return a value.
  }
}
