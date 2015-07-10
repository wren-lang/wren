class A {}

class B is A {
  this new() {
    super // expect error
  }
}
