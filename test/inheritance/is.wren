class A {}
class B is A {}
class C is B {}
var a = new A
var b = new B
var c = new C

io.write(a is A) // expect: true
io.write(a is B) // expect: false
io.write(a is C) // expect: false
io.write(b is A) // expect: true
io.write(b is B) // expect: true
io.write(b is C) // expect: false
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true
