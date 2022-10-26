class Foo {
  static []                                                -> String { "static 0-subscript []" }
  static [a: String]                                       -> String { "static 1-subscript [%(a)]" }
  static [a: String, b: String]                            -> String { "static 2-subscript [%(a), %(b)]" }
  static [a: String, b: String, c: String]                 -> String { "static 3-subscript [%(a), %(b), %(c)]" }
  static []=(value: String)                                -> String { "static 0-subscript setter [] = %(value)" }
  static [a: String]=(value: String)                       -> String { "static 1-subscript setter [%(a)] = %(value)" }
  static [a: String, b: String]=(value: String)            -> String { "static 2-subscript setter [%(a), %(b)] = %(value)" }
  static [a: String, b: String, c: String]=(value: String) -> String { "static 3-subscript setter [%(a), %(b), %(c)] = %(value)" }

  static method                                            -> String { "static method" }
  static method()                                          -> String { "static method()" }
  static method(a: String)                                 -> String { "static method(%(a))" }
  static method(a: String, b: String)                      -> String { "static method(%(a), %(b))" }
  static method(a: String, b: String, c: String)           -> String { "static method(%(a), %(b), %(c))" }

  construct new() -> Foo {}

  []                                                       -> String { "0-subscript []" }
  [a: String]                                              -> String { "1-subscript [%(a)]" }
  [a: String, b: String]                                   -> String { "2-subscript [%(a), %(b)]" }
  [a: String, b: String, c: String]                        -> String { "3-subscript [%(a), %(b), %(c)]" }
  []=(value: String)                                       -> String { "0-subscript setter [] = %(value)" }
  [a: String]=(value: String)                              -> String { "1-subscript setter [%(a)] = %(value)" }
  [a: String, b: String]=(value: String)                   -> String { "2-subscript setter [%(a), %(b)] = %(value)" }
  [a: String, b: String, c: String]=(value: String)        -> String { "3-subscript setter [%(a), %(b), %(c)] = %(value)" }

  method                                                   -> String { "method" }
  method()                                                 -> String { "method()" }
  method(a: String)                                        -> String { "method(%(a))" }
  method(a: String, b: String)                             -> String { "method(%(a), %(b))" }
  method(a: String, b: String, c: String)                  -> String { "method(%(a), %(b), %(c))" }
}

class Bar {
  static bar = a: String                                   -> String { "static 1-setter bar = (%(a))" }

  construct new() {}
  bar = a: String                                          -> String { "1-setter bar = (%(a))" }
}

System.print(Foo[])                        // expect: static 0-subscript []
System.print(Foo["a"])                     // expect: static 1-subscript [a]
System.print(Foo["a", "b"])                // expect: static 2-subscript [a, b]
System.print(Foo["a", "b", "c"])           // expect: static 3-subscript [a, b, c]
System.print(Foo[] = "value")              // expect: static 0-subscript setter [] = value
System.print(Foo["a"] = "value")           // expect: static 1-subscript setter [a] = value
System.print(Foo["a", "b"] = "value")      // expect: static 2-subscript setter [a, b] = value
System.print(Foo["a", "b", "c"] = "value") // expect: static 3-subscript setter [a, b, c] = value

System.print(Foo.method)                   // expect: static method
System.print(Foo.method())                 // expect: static method()
System.print(Foo.method("a"))              // expect: static method(a)
System.print(Foo.method("a", "b"))         // expect: static method(a, b)
System.print(Foo.method("a", "b", "c"))    // expect: static method(a, b, c)

System.print(Bar.bar = "a") // expect: static 1-setter bar = (a)

var foo = Foo.new()
System.print(foo[])                        // expect: 0-subscript []
System.print(foo["a"])                     // expect: 1-subscript [a]
System.print(foo["a", "b"])                // expect: 2-subscript [a, b]
System.print(foo["a", "b", "c"])           // expect: 3-subscript [a, b, c]
System.print(foo[] = "value")              // expect: 0-subscript setter [] = value
System.print(foo["a"] = "value")           // expect: 1-subscript setter [a] = value
System.print(foo["a", "b"] = "value")      // expect: 2-subscript setter [a, b] = value
System.print(foo["a", "b", "c"] = "value") // expect: 3-subscript setter [a, b, c] = value

System.print(foo.method)                   // expect: method
System.print(foo.method())                 // expect: method()
System.print(foo.method("a"))              // expect: method(a)
System.print(foo.method("a", "b"))         // expect: method(a, b)
System.print(foo.method("a", "b", "c"))    // expect: method(a, b, c)

var bar = Bar.new()
System.print(bar.bar = "a") // expect: 1-setter bar = (a)
