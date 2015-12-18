class A {}

class B is A {
  def construct new() {
    super // expect error
  }
}
