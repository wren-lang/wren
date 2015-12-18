class Foo {
  static def initialize { __field = "Foo field" }

  static def closeOverGet {
    return Fn.new { __field }
  }

  static def closeOverSet {
    return Fn.new { __field = "new value" }
  }
}

Foo.initialize
System.print(Foo.closeOverGet.call()) // expect: Foo field
Foo.closeOverSet.call()
System.print(Foo.closeOverGet.call()) // expect: new value
