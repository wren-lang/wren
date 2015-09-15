class Foo {
  static initialize { __field = "Foo field" }

  static closeOverGet {
    return Fn.new { __field }
  }

  static closeOverSet {
    return Fn.new { __field = "new value" }
  }
}

Foo.initialize
System.print(Foo.closeOverGet.call()) // expect: Foo field
Foo.closeOverSet.call()
System.print(Foo.closeOverGet.call()) // expect: new value
