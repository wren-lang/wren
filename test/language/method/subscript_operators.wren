class Foo {
  static []                { "static 0-subscript []" }
  static [a]               { "static 1-subscript [%(a)]" }
  static [a, b]            { "static 2-subscript [%(a), %(b)]" }
  static [a, b, c]         { "static 3-subscript [%(a), %(b), %(c)]" }
  static []=(value)        { "static 0-subscript setter [] = %(value)" }
  static [a]=(value)       { "static 1-subscript setter [%(a)] = %(value)" }
  static [a, b]=(value)    { "static 2-subscript setter [%(a), %(b)] = %(value)" }
  static [a, b, c]=(value) { "static 3-subscript setter [%(a), %(b), %(c)] = %(value)" }

  construct new() {}
  []                { "0-subscript []" }
  [a]               { "1-subscript [%(a)]" }
  [a, b]            { "2-subscript [%(a), %(b)]" }
  [a, b, c]         { "3-subscript [%(a), %(b), %(c)]" }
  []=(value)        { "0-subscript setter [] = %(value)" }
  [a]=(value)       { "1-subscript setter [%(a)] = %(value)" }
  [a, b]=(value)    { "2-subscript setter [%(a), %(b)] = %(value)" }
  [a, b, c]=(value) { "3-subscript setter [%(a), %(b), %(c)] = %(value)" }
}

System.print(Foo[])                        // expect: static 0-subscript []
System.print(Foo["a"])                     // expect: static 1-subscript [a]
System.print(Foo["a", "b"])                // expect: static 2-subscript [a, b]
System.print(Foo["a", "b", "c"])           // expect: static 3-subscript [a, b, c]
System.print(Foo[] = "value")              // expect: static 0-subscript setter [] = value
System.print(Foo["a"] = "value")           // expect: static 1-subscript setter [a] = value
System.print(Foo["a", "b"] = "value")      // expect: static 2-subscript setter [a, b] = value
System.print(Foo["a", "b", "c"] = "value") // expect: static 3-subscript setter [a, b, c] = value

var foo = Foo.new()
System.print(foo[])                        // expect: 0-subscript []
System.print(foo["a"])                     // expect: 1-subscript [a]
System.print(foo["a", "b"])                // expect: 2-subscript [a, b]
System.print(foo["a", "b", "c"])           // expect: 3-subscript [a, b, c]
System.print(foo[] = "value")              // expect: 0-subscript setter [] = value
System.print(foo["a"] = "value")           // expect: 1-subscript setter [a] = value
System.print(foo["a", "b"] = "value")      // expect: 2-subscript setter [a, b] = value
System.print(foo["a", "b", "c"] = "value") // expect: 3-subscript setter [a, b, c] = value
