class Foo {
  def construct new() {}
  def bar { "on instance" }
  def static bar { "on metaclass" }

  def bar(arg) { "on instance %(arg)" }
  def static bar(arg) { "on metaclass %(arg)" }
}

System.print(Foo.new().bar)        // expect: on instance
System.print(Foo.bar)              // expect: on metaclass
System.print(Foo.new().bar("arg")) // expect: on instance arg
System.print(Foo.bar("arg"))       // expect: on metaclass arg
