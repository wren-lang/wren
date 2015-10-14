var f0 = Fn.new { System.print("zero") }
var f1 = Fn.new {|a| System.print("one %(a)") }
var f2 = Fn.new {|a, b| System.print("two %(a) %(b)") }
var f3 = Fn.new {|a, b, c| System.print("three %(a) %(b) %(c)") }

f0.call("a") // expect: zero
f0.call("a", "b") // expect: zero

f1.call("a", "b") // expect: one a
f1.call("a", "b", "c") // expect: one a

f2.call("a", "b", "c") // expect: two a b
f2.call("a", "b", "c", "d") // expect: two a b

f3.call("a", "b", "c", "d") // expect: three a b c
f3.call("a", "b", "c", "d", "e") // expect: three a b c
