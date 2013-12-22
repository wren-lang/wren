class A {}
class B is A {}
class C is B {}
var a = new A
var b = new B
var c = new C

IO.write(a is A) // expect: true
IO.write(a is B) // expect: false
IO.write(a is C) // expect: false
IO.write(b is A) // expect: true
IO.write(b is B) // expect: true
IO.write(b is C) // expect: false
IO.write(c is A) // expect: true
IO.write(c is B) // expect: true
IO.write(c is C) // expect: true
