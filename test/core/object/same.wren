// Value types compare by value.
IO.print(Object.same(true, true)) // expect: true
IO.print(Object.same(true, false)) // expect: false

IO.print(Object.same(null, null)) // expect: true

IO.print(Object.same(1 + 2, 2 + 1)) // expect: true
IO.print(Object.same(1 + 2, 2 + 2)) // expect: false

IO.print(Object.same(1..2, 1..2)) // expect: true
IO.print(Object.same(1..2, 1..3)) // expect: false

IO.print(Object.same("ab", "a" + "b")) // expect: true
IO.print(Object.same("ab", "a" + "c")) // expect: false

// Different types are never the same.
IO.print(Object.same(null, false)) // expect: false
IO.print(Object.same(true, 2)) // expect: false
IO.print(Object.same(1..2, 2)) // expect: false
IO.print(Object.same("1", 1)) // expect: false

// Classes compare by identity.
IO.print(Object.same(Bool, Num)) // expect: false
IO.print(Object.same(Bool, Bool)) // expect: true

// Other types compare by identity.
class Foo {}

var foo = Foo.new()
IO.print(Object.same(foo, foo)) // expect: true
IO.print(Object.same(foo, Foo.new())) // expect: false

// Ignores == operators.
class Bar {
  ==(other) { true }
}

var bar = Bar.new()
IO.print(Object.same(bar, bar)) // expect: true
IO.print(Object.same(bar, Bar.new())) // expect: false
