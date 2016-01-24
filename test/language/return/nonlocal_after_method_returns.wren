class Foo {
  static setup() {
    __field = Fn.new {
      return // expect runtime error: Non-local return from method that already returned.
    }
  }

  static test() {
    __field.call()
  }
}

Foo.setup()
System.print(Foo.test())
