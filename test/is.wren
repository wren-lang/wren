io.write(Num is Class) // expect: true
io.write(true is Bool) // expect: true
io.write((fn 1) is Function) // expect: true
io.write(123 is Num) // expect: true
io.write(null is Null) // expect: true
io.write("s" is String) // expect: true

io.write(Num is Bool) // expect: false
io.write(null is Class) // expect: false
io.write(true is Function) // expect: false
io.write((fn 1) is Num) // expect: false
io.write("s" is Null) // expect: false
io.write(123 is String) // expect: false

// Everything extends Object.
io.write(Num is Object) // expect: true
io.write(null is Object) // expect: true
io.write(true is Object) // expect: true
io.write((fn 1) is Object) // expect: true
io.write("s" is Object) // expect: true
io.write(123 is Object) // expect: true

// TODO(bob): Non-class on RHS.
// TODO(bob): Precedence and associativity.
// TODO(bob): Metaclasses ("Num is Class").
