// Value types compare by value.
System.print(Object.same(true, true)) // expect: true
System.print(Object.same(true, false)) // expect: false

System.print(Object.same(null, null)) // expect: true

System.print(Object.same(1 + 2, 2 + 1)) // expect: true
System.print(Object.same(1 + 2, 2 + 2)) // expect: false

System.print(Object.same(1..2, 1..2)) // expect: true
System.print(Object.same(1..2, 1..3)) // expect: false

System.print(Object.same("ab", "a" + "b")) // expect: true
System.print(Object.same("ab", "a" + "c")) // expect: false

// Different types are never the same.
System.print(Object.same(null, false)) // expect: false
System.print(Object.same(true, 2)) // expect: false
System.print(Object.same(1..2, 2)) // expect: false
System.print(Object.same("1", 1)) // expect: false

// Classes compare by identity.
System.print(Object.same(Bool, Num)) // expect: false
System.print(Object.same(Bool, Bool)) // expect: true

// Other types compare by identity.
class Foo {
  construct new() {}
}

var foo = Foo.new()
System.print(Object.same(foo, foo)) // expect: true
System.print(Object.same(foo, Foo.new())) // expect: false

// Ignores == operators.
class Bar {
  construct new() {}
  ==(other) { true }
}

var bar = Bar.new()
System.print(Object.same(bar, bar)) // expect: true
System.print(Object.same(bar, Bar.new())) // expect: false
