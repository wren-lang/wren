class A {}
class B is A {}
class C is B {}
var a = A.new()
var b = B.new()
var c = C.new()

IO.print(a is A) // expect: true
IO.print(a is B) // expect: false
IO.print(a is C) // expect: false
IO.print(b is A) // expect: true
IO.print(b is B) // expect: true
IO.print(b is C) // expect: false
IO.print(c is A) // expect: true
IO.print(c is B) // expect: true
IO.print(c is C) // expect: true
