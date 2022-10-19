class Foo {
  static foo = (a)         { "static 1-setter foo = (%(a))" }
  static [subscript] = (a) { "static 1-subscript setter [%(subscript)] = (%(a))" }

  construct new() {}
  foo = (a)         { "1-setter foo = (%(a))" }
  [subscript] = (a) { "1-subscript setter [%(subscript)] = (%(a))" }
}

System.print(Foo.foo = "a")            // expect: static 1-setter foo = (a)
System.print(Foo.foo = ("a"))          // expect: static 1-setter foo = (a)
System.print(Foo["subscript"] = "a")   // expect: static 1-subscript setter [subscript] = (a)
System.print(Foo["subscript"] = ("a")) // expect: static 1-subscript setter [subscript] = (a)

var foo = Foo.new()
System.print(foo.foo = "a")            // expect: 1-setter foo = (a)
System.print(foo.foo = ("a"))          // expect: 1-setter foo = (a)
System.print(foo["subscript"] = "a")   // expect: 1-subscript setter [subscript] = (a)
System.print(foo["subscript"] = ("a")) // expect: 1-subscript setter [subscript] = (a)
