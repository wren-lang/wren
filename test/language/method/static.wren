class Foo {
  bar { "on instance" }
  static bar { "on metaclass" }

  bar(arg) { "on instance " + arg }
  static bar(arg) { "on metaclass " + arg }
}

IO.print(Foo.new().bar)        // expect: on instance
IO.print(Foo.bar)              // expect: on metaclass
IO.print(Foo.new().bar("arg")) // expect: on instance arg
IO.print(Foo.bar("arg"))       // expect: on metaclass arg
