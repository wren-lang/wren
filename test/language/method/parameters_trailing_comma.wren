class Foo {
  static method(a, b,) { System.print("%(a) %(b)") }
}

Foo.method(1, 2) // expect: 1 2
