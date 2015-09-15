class Foo {
  construct new() {}
  static bar { true }
  static baz { 1 }
}

// Condition precedence.
System.print(true ? 1 : 2) // expect: 1
System.print((true) ? 1 : 2) // expect: 1
System.print([true][0] ? 1 : 2) // expect: 1
System.print(Foo.bar ? 1 : 2) // expect: 1
System.print(3..4 ? 1 : 2) // expect: 1
System.print(3 * 4 ? 1 : 2) // expect: 1
System.print(3 + 4 ? 1 : 2) // expect: 1
System.print(true || false ? 1 : 2) // expect: 1
System.print(!false ? 1 : 2) // expect: 1
System.print(~0 ? 1 : 2) // expect: 1
System.print(3 is Num ? 1 : 2) // expect: 1
System.print(Foo.new() ? 1 : 2) // expect: 1

var a = 0
System.print(a = 3 ? 1 : 2) // expect: 1
System.print(a) // expect: 1

// Then branch precedence.
System.print(true ? (1) : 2) // expect: 1
System.print(true ? [1][0] : 2) // expect: 1
System.print(true ? Foo.baz : 2) // expect: 1
System.print(true ? 3..4 : 2) // expect: 3..4
System.print(true ? 3 * 4 : 2) // expect: 12
System.print(true ? 3 + 4 : 2) // expect: 7
System.print(true ? 1 || false : 2) // expect: 1
System.print(true ? !true : 2) // expect: false
System.print(true ? ~0 : 2) // expect: 4294967295
System.print(true ? 3 is Bool : 2) // expect: false
System.print(true ? Foo.new() : 2) // expect: instance of Foo

System.print(true ? a = 5 : 2) // expect: 5
System.print(a) // expect: 5

// Else branch precedence.
System.print(false ? 1 : (2)) // expect: 2
System.print(false ? 1 : [2][0]) // expect: 2
System.print(false ? 2 : Foo.baz) // expect: 1
System.print(false ? 1 : 3..4) // expect: 3..4
System.print(false ? 1 : 3 * 4) // expect: 12
System.print(false ? 1 : 3 + 4) // expect: 7
System.print(false ? 1 : 2 || false) // expect: 2
System.print(false ? 1 : !false) // expect: true
System.print(false ? 1 : ~0) // expect: 4294967295
System.print(false ? 1 : 3 is Num) // expect: true
System.print(false ? 1 : Foo.new()) // expect: instance of Foo

// Associativity.
System.print(true ? 2 : true ? 4 : 5) // expect: 2
System.print(false ? 2 : true ? 4 : 5) // expect: 4
