class Foo {
  bar { return "on instance" }
  static bar { return "on metaclass" }

  bar(arg) { return "on instance " + arg }
  static bar(arg) { return "on metaclass " + arg }
}

io.write((new Foo).bar)        // expect: on instance
io.write(Foo.bar)              // expect: on metaclass
io.write((new Foo).bar("arg")) // expect: on instance arg
io.write(Foo.bar("arg"))       // expect: on metaclass arg
