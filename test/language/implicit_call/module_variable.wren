def f1(arg) { System.print(arg) }
f1("string") // expect: string

def f2(block) { System.print(block()) }
f2 { "block" } // expect: block

def f3(a, b, c) { System.print("%(a) %(b) %(c())") }
f3(1, 2) { 3 } // expect: 1 2 3
