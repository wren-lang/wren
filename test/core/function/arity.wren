System.print(fn () {}.arity) // expect: 0
System.print(fn (a) {a}.arity) // expect: 1
System.print(fn (a, b) {a}.arity) // expect: 2
System.print(fn (a, b, c) {a}.arity) // expect: 3
System.print(fn (a, b, c, d) {a}.arity) // expect: 4
