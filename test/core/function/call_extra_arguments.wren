def f0() { System.print("zero") }
def f1(a) { System.print("one %(a)") }
def f2(a, b) { System.print("two %(a) %(b)") }
def f3(a, b, c) { System.print("three %(a) %(b) %(c)") }

f0("a") // expect: zero
f0("a", "b") // expect: zero

f1("a", "b") // expect: one a
f1("a", "b", "c") // expect: one a

f2("a", "b", "c") // expect: two a b
f2("a", "b", "c", "d") // expect: two a b

f3("a", "b", "c", "d") // expect: three a b c
f3("a", "b", "c", "d", "e") // expect: three a b c
