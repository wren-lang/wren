class Foo {
  construct new() {}
  bar { "on instance" }
  static bar { "on metaclass" }

  bar(arg) { "on instance %(arg)" }
  static bar(arg) { "on metaclass %(arg)" }
}

System.print(Foo.new().bar)        // expect: on instance
System.print(Foo.bar)              // expect: on metaclass
System.print(Foo.new().bar("arg")) // expect: on instance arg
System.print(Foo.bar("arg"))       // expect: on metaclass arg
