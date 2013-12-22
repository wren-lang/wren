class Foo {
  bar { return "on instance" }
  static bar { return "on metaclass" }

  bar(arg) { return "on instance " + arg }
  static bar(arg) { return "on metaclass " + arg }
}

IO.write((new Foo).bar)        // expect: on instance
IO.write(Foo.bar)              // expect: on metaclass
IO.write((new Foo).bar("arg")) // expect: on instance arg
IO.write(Foo.bar("arg"))       // expect: on metaclass arg
