IO.print(Num is Class) // expect: true
IO.print(true is Bool) // expect: true
IO.print((fn 1) is Function) // expect: true
IO.print(123 is Num) // expect: true
IO.print(null is Null) // expect: true
IO.print("s" is String) // expect: true

IO.print(Num is Bool) // expect: false
IO.print(null is Class) // expect: false
IO.print(true is Function) // expect: false
IO.print((fn 1) is Num) // expect: false
IO.print("s" is Null) // expect: false
IO.print(123 is String) // expect: false

// Everything extends Object.
IO.print(Num is Object) // expect: true
IO.print(null is Object) // expect: true
IO.print(true is Object) // expect: true
IO.print((fn 1) is Object) // expect: true
IO.print("s" is Object) // expect: true
IO.print(123 is Object) // expect: true

// Classes extend Class.
IO.print(Num is Class) // expect: true
IO.print(null is Class) // expect: false
IO.print(true is Class) // expect: false
IO.print((fn 1) is Class) // expect: false
IO.print("s" is Class) // expect: false
IO.print(123 is Class) // expect: false

// TODO: Non-class on RHS.
// TODO: Precedence and associativity.
