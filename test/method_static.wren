class Foo {
  bar { "on instance" }
  static bar { "on metaclass" }

  bar(arg) { "on instance " + arg }
  static bar(arg) { "on metaclass " + arg }
}

io.write("on metaclass " + "arg") // expect: on metaclass arg
io.write(Foo.new.bar)        // expect: on instance
io.write(Foo.bar)            // expect: on metaclass
io.write(Foo.new.bar("arg")) // expect: on instance arg
io.write(Foo.bar("arg"))     // expect: on metaclass arg
