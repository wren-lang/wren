var f0 = fn IO.print("zero")
var f1 = fn(a) IO.print("one " + a)
var f2 = fn(a, b) IO.print("two " + a + " " + b)
var f3 = fn(a, b, c) IO.print("three " + a + " " + b + " " + c)

f0.call("a") // expect: zero
f0.call("a", "b") // expect: zero

f1.call("a", "b") // expect: one a
f1.call("a", "b", "c") // expect: one a

f2.call("a", "b", "c") // expect: two a b
f2.call("a", "b", "c", "d") // expect: two a b

f3.call("a", "b", "c", "d") // expect: three a b c
f3.call("a", "b", "c", "d", "e") // expect: three a b c
