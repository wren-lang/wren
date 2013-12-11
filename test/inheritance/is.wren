class A {}
class B is A {}
class C is B {}
var a = A.new
var b = B.new
var c = C.new

io.write(a is A) // expect: true
io.write(a is B) // expect: false
io.write(a is C) // expect: false
io.write(b is A) // expect: true
io.write(b is B) // expect: true
io.write(b is C) // expect: false
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true
