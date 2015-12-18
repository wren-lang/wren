class Foo {
  def static initialize { __field = "Foo field" }

  def static closeOverGet {
    return Fn.new { __field }
  }

  def static closeOverSet {
    return Fn.new { __field = "new value" }
  }
}

Foo.initialize
System.print(Foo.closeOverGet.call()) // expect: Foo field
Foo.closeOverSet.call()
System.print(Foo.closeOverGet.call()) // expect: new value
