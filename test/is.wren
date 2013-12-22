IO.write(Num is Class) // expect: true
IO.write(true is Bool) // expect: true
IO.write((fn 1) is Function) // expect: true
IO.write(123 is Num) // expect: true
IO.write(null is Null) // expect: true
IO.write("s" is String) // expect: true

IO.write(Num is Bool) // expect: false
IO.write(null is Class) // expect: false
IO.write(true is Function) // expect: false
IO.write((fn 1) is Num) // expect: false
IO.write("s" is Null) // expect: false
IO.write(123 is String) // expect: false

// Everything extends Object.
IO.write(Num is Object) // expect: true
IO.write(null is Object) // expect: true
IO.write(true is Object) // expect: true
IO.write((fn 1) is Object) // expect: true
IO.write("s" is Object) // expect: true
IO.write(123 is Object) // expect: true

// Classes extend Class.
IO.write(Num is Class) // expect: true
IO.write(null is Class) // expect: false
IO.write(true is Class) // expect: false
IO.write((fn 1) is Class) // expect: false
IO.write("s" is Class) // expect: false
IO.write(123 is Class) // expect: false

// TODO: Non-class on RHS.
// TODO: Precedence and associativity.
