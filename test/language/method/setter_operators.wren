class Foo {
  static foo = ()                { "static 0-setter foo = ()" }
  static foo = (a)               { "static 1-setter foo = (%(a))" }
  static foo = (a, b)            { "static 2-setter foo = (%(a), %(b))" }
  static foo = (a, b, c)         { "static 3-setter foo = (%(a), %(b), %(c))" }
  static [subscript] = ()        { "static 0-subscript setter [%(subscript)] = ()" }
  static [subscript] = (a)       { "static 1-subscript setter [%(subscript)] = (%(a))" }
  static [subscript] = (a, b)    { "static 2-subscript setter [%(subscript)] = (%(a), %(b))" }
  static [subscript] = (a, b, c) { "static 3-subscript setter [%(subscript)] = (%(a), %(b), %(c))" }

  construct new() {}
  foo = ()                       { "0-setter foo = ()" }
  foo = (a)                      { "1-setter foo = (%(a))" }
  foo = (a, b)                   { "2-setter foo = (%(a), %(b))" }
  foo = (a, b, c)                { "3-setter foo = (%(a), %(b), %(c))" }
  [subscript] = ()               { "0-subscript setter [%(subscript)] = ()" }
  [subscript] = (a)              { "1-subscript setter [%(subscript)] = (%(a))" }
  [subscript] = (a, b)           { "2-subscript setter [%(subscript)] = (%(a), %(b))" }
  [subscript] = (a, b, c)        { "3-subscript setter [%(subscript)] = (%(a), %(b), %(c))" }
}

class Bar {
  static bar = a                 { "static 1-setter bar = (%(a))" }

  construct new() {}
  bar = a                        { "1-setter bar = (%(a))" }
}

System.print(Foo.foo = ())                       // expect: static 0-setter foo = ()
System.print(Foo.foo = "a")                      // expect: static 1-setter foo = (a)
System.print(Foo.foo = ("a"))                    // expect: static 1-setter foo = (a)
System.print(Foo.foo = ("a", "b"))               // expect: static 2-setter foo = (a, b)
System.print(Foo.foo = ("a", "b", "c"))          // expect: static 3-setter foo = (a, b, c)
System.print(Foo["subscript"] = ())              // expect: static 0-subscript setter [subscript] = ()
System.print(Foo["subscript"] = "a")             // expect: static 1-subscript setter [subscript] = (a)
System.print(Foo["subscript"] = ("a"))           // expect: static 1-subscript setter [subscript] = (a)
System.print(Foo["subscript"] = ("a", "b"))      // expect: static 2-subscript setter [subscript] = (a, b)
System.print(Foo["subscript"] = ("a", "b", "c")) // expect: static 3-subscript setter [subscript] = (a, b, c)

System.print(Bar.bar = "a")                      // expect: static 1-setter bar = (a)

var foo = Foo.new()
System.print(foo.foo = ())                       // expect: 0-setter foo = ()
System.print(foo.foo = "a")                      // expect: 1-setter foo = (a)
System.print(foo.foo = ("a"))                    // expect: 1-setter foo = (a)
System.print(foo.foo = ("a", "b"))               // expect: 2-setter foo = (a, b)
System.print(foo.foo = ("a", "b", "c"))          // expect: 3-setter foo = (a, b, c)
System.print(foo["subscript"] = ())              // expect: 0-subscript setter [subscript] = ()
System.print(foo["subscript"] = "a")             // expect: 1-subscript setter [subscript] = (a)
System.print(foo["subscript"] = ("a"))           // expect: 1-subscript setter [subscript] = (a)
System.print(foo["subscript"] = ("a", "b"))      // expect: 2-subscript setter [subscript] = (a, b)
System.print(foo["subscript"] = ("a", "b", "c")) // expect: 3-subscript setter [subscript] = (a, b, c)

var bar = Bar.new()
System.print(bar.bar = "a")                      // expect: 1-setter bar = (a)
