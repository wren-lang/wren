class Base {
  def foo { System.print("Base.foo") }
  def foo(a) { System.print("Base.foo(a)") }
  def foo(a, b) { System.print("Base.foo(a, b)") }
}

class Derived is Base {
  construct new() {}

  def foo(a) {
    System.print("Derived.bar(a)")
    super
    super(1)
    super(1, 2)
  }
}

Derived.new().foo(1)
// expect: Derived.bar(a)
// expect: Base.foo
// expect: Base.foo(a)
// expect: Base.foo(a, b)
