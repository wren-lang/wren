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

fn () {
  Nonlocal = "fn"
}()
System.print(Nonlocal) // expect: fn
