var Nonlocal = "before"
System.print(Nonlocal) // expect: before
Nonlocal = "after"
System.print(Nonlocal) // expect: after

class Foo {
  static method {
    Nonlocal = "method"
  }
}

Foo.method
System.print(Nonlocal) // expect: method

Fn.new {
  Nonlocal = "fn"
}.call()
System.print(Nonlocal) // expect: fn
