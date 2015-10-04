class Foo {
  static name {
    System.print("Foo.name") // expect: Foo.name
    System.print(super)      // expect: Foo
  }
}

Foo.name
