var Nonlocal = "before"
IO.print(Nonlocal) // expect: before
Nonlocal = "after"
IO.print(Nonlocal) // expect: after

class Foo {
  static method {
    Nonlocal = "method"
  }
}

Foo.method
IO.print(Nonlocal) // expect: method

new Fn {
  Nonlocal = "fn"
}.call()
IO.print(Nonlocal) // expect: fn
