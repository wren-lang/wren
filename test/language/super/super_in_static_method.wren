class Foo {
  static def name {
    System.print("Foo.name") // expect: Foo.name
    System.print(super)      // expect: Foo
  }
}

Foo.name
