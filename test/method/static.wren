class Foo {
  bar { return "on instance" }
  static bar { return "on metaclass" }

  bar(arg) { return "on instance " + arg }
  static bar(arg) { return "on metaclass " + arg }
}

IO.print((new Foo).bar)        // expect: on instance
IO.print(Foo.bar)              // expect: on metaclass
IO.print((new Foo).bar("arg")) // expect: on instance arg
IO.print(Foo.bar("arg"))       // expect: on metaclass arg
