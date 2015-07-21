class A {}

class B is A {
  construct new() {
    super // expect error
  }
}
