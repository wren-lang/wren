
class Foo {
  static [a,]             { "static 1-subscript [%(a)]"}
  static [a, b,]          { "static 2-subscript [%(a), %(b)]"}
  static [a, b, c,]       { "static 3-subscript [%(a), %(b), %(c)]"}
  static foo(a,)          { "static foo(%(a))" }
  static foo(a, b,)       { "static foo(%(a), %(b))" }
  static foo(a, b, c,)    { "static foo(%(a), %(b), %(c))" }

  construct new(a,)       { System.print("construct new(%(a))",) }
  construct new(a, b,)    { System.print("construct new(%(a), %(b))",) }
  construct new(a, b, c,) { System.print("construct new(%(a), %(b), %(c))",) }
  [a,]                    { "1-subscript [%(a)]"}
  [a, b,]                 { "2-subscript [%(a), %(b)]"}
  [a, b, c,]              { "3-subscript [%(a), %(b), %(c)]"}
  foo(a,)                 { "foo(%(a))" }
  foo(a, b,)              { "foo(%(a), %(b))" }
  foo(a, b, c,)           { "foo(%(a), %(b), %(c))" }
}

System.print(Foo["a",],)               // expect: static 1-subscript [a]
System.print(Foo["a", "b",],)          // expect: static 2-subscript [a, b]
System.print(Foo["a", "b", "c",],)     // expect: static 3-subscript [a, b, c]
System.print(Foo.foo("a",),)           // expect: static foo(a)
System.print(Foo.foo("a", "b",),)      // expect: static foo(a, b)
System.print(Foo.foo("a", "b", "c",),) // expect: static foo(a, b, c)

var foo
foo = Foo.new("a",)                    // expect: construct new(a)
foo = Foo.new("a", "b",)               // expect: construct new(a, b)
foo = Foo.new("a", "b", "c",)          // expect: construct new(a, b, c)
System.print(foo["a",],)               // expect: 1-subscript [a]
System.print(foo["a", "b",],)          // expect: 2-subscript [a, b]
System.print(foo["a", "b", "c",],)     // expect: 3-subscript [a, b, c]
System.print(foo.foo("a",),)           // expect: foo(a)
System.print(foo.foo("a", "b",),)      // expect: foo(a, b)
System.print(foo.foo("a", "b", "c",),) // expect: foo(a, b, c)
