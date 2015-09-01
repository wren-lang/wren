class Foo {
  construct new() {}
  static bar { true }
  static baz { 1 }
}

// Condition precedence.
IO.print(true ? 1 : 2) // expect: 1
IO.print((true) ? 1 : 2) // expect: 1
IO.print([true][0] ? 1 : 2) // expect: 1
IO.print(Foo.bar ? 1 : 2) // expect: 1
IO.print(3..4 ? 1 : 2) // expect: 1
IO.print(3 * 4 ? 1 : 2) // expect: 1
IO.print(3 + 4 ? 1 : 2) // expect: 1
IO.print(true || false ? 1 : 2) // expect: 1
IO.print(!false ? 1 : 2) // expect: 1
IO.print(~0 ? 1 : 2) // expect: 1
IO.print(3 is Num ? 1 : 2) // expect: 1
IO.print(Foo.new() ? 1 : 2) // expect: 1

var a = 0
IO.print(a = 3 ? 1 : 2) // expect: 1
IO.print(a) // expect: 1

// Then branch precedence.
IO.print(true ? (1) : 2) // expect: 1
IO.print(true ? [1][0] : 2) // expect: 1
IO.print(true ? Foo.baz : 2) // expect: 1
IO.print(true ? 3..4 : 2) // expect: 3..4
IO.print(true ? 3 * 4 : 2) // expect: 12
IO.print(true ? 3 + 4 : 2) // expect: 7
IO.print(true ? 1 || false : 2) // expect: 1
IO.print(true ? !true : 2) // expect: false
IO.print(true ? ~0 : 2) // expect: 4294967295
IO.print(true ? 3 is Bool : 2) // expect: false
IO.print(true ? Foo.new() : 2) // expect: instance of Foo

IO.print(true ? a = 5 : 2) // expect: 5
IO.print(a) // expect: 5

// Else branch precedence.
IO.print(false ? 1 : (2)) // expect: 2
IO.print(false ? 1 : [2][0]) // expect: 2
IO.print(false ? 2 : Foo.baz) // expect: 1
IO.print(false ? 1 : 3..4) // expect: 3..4
IO.print(false ? 1 : 3 * 4) // expect: 12
IO.print(false ? 1 : 3 + 4) // expect: 7
IO.print(false ? 1 : 2 || false) // expect: 2
IO.print(false ? 1 : !false) // expect: true
IO.print(false ? 1 : ~0) // expect: 4294967295
IO.print(false ? 1 : 3 is Num) // expect: true
IO.print(false ? 1 : Foo.new()) // expect: instance of Foo

// Associativity.
IO.print(true ? 2 : true ? 4 : 5) // expect: 2
IO.print(false ? 2 : true ? 4 : 5) // expect: 4
